% roofline_model.m — standalone Roofline + Amdahl's-Law model
% (Project 1: Roofline Reckoning, CPEN 315/733, Group 3)
%
% Owner: Obed Ninson (Python/MATLAB + Test/Docs Lead)
%
% Independent hand-worked cross-check of the Python pipeline
% (analyze.py/plots.py), not a re-implementation driven by the same code
% path — this is deliberately the "do the CPU-time/Amdahl worked example
% by hand" step called for in ai_use_declaration/ai_use_log.md's action
% item #2, so it takes peak_flops.c / stream_triad.c's measured numbers
% and gh_bench's measured (AI, GFLOP/s, phase1/phase2) points as plain
% inputs below rather than reading the JSONL programmatically.
%
% Fill in PEAK_GFLOPS / PEAK_BANDWIDTH_GBS from your own machine's
% peak_flops / stream_triad run output, and the KERNELS table from your
% own results/summary.csv, before trusting the numbers this prints.

clear; clc;

%% ---- Measured roofs (fill in from your own run) ------------------------
PEAK_GFLOPS       = 0;   % from peak_flops.c stdout: "gflops": ...
PEAK_BANDWIDTH_GBS = 0;  % from stream_triad.c stdout: "bandwidth_gbs": ...

if PEAK_GFLOPS <= 0 || PEAK_BANDWIDTH_GBS <= 0
    warning(['PEAK_GFLOPS / PEAK_BANDWIDTH_GBS are unset (0). Run ' ...
             './peak_flops' newline './stream_triad' newline ...
             'and fill in their measured values before trusting this plot.']);
end

ridge_ai = PEAK_GFLOPS / PEAK_BANDWIDTH_GBS;  % FLOP/byte at the roofline corner

%% ---- Measured kernel points (fill in from results/summary.csv) ---------
% One row per (kernel, config) you want to plot. phase1/phase2 are the
% same serial/parallel split gh_bench.c's harness reports; leave a row's
% phase1/phase2 at 0 if that kernel doesn't report sub-phase timing.
kernels = struct( ...
    'name',    {'momo_match', 'cocobod_yield_regression', 'rainfall_interpolate'}, ...
    'ai',      {0, 0, 0}, ...            % arithmetic_intensity from summary.csv
    'gflops',  {0, 0, 0}, ...            % median_gflops from summary.csv
    'phase1_s', {0, 0, 0}, ...           % serial-phase seconds
    'phase2_s', {0, 0, 0});              % parallel-phase seconds

%% ---- Roofline plot -------------------------------------------------------
figure('Name', 'Roofline Model', 'Color', 'w');
hold on;

ai_span = logspace(-4, 2, 200);
compute_roof = min(PEAK_GFLOPS, ai_span * PEAK_BANDWIDTH_GBS);
plot(ai_span, compute_roof, 'k-', 'LineWidth', 1.5, 'DisplayName', 'Roofline bound');
xline(ridge_ai, ':', sprintf('ridge AI = %.4f', ridge_ai), 'Color', [0.5 0.5 0.5]);

markers = {'o', 's', '^', 'd', 'v', 'p'};
for i = 1:numel(kernels)
    k = kernels(i);
    if k.ai <= 0 || k.gflops <= 0
        continue  % unfilled placeholder row — skip rather than plot a fake (0,0) point
    end
    attainable = min(PEAK_GFLOPS, k.ai * PEAK_BANDWIDTH_GBS);
    pct = 100 * k.gflops / attainable;
    if k.ai < ridge_ai
        region = 'memory-bound';
    else
        region = 'compute-bound';
    end
    fprintf('%-28s AI=%.6f  GFLOP/s=%.4f  attainable=%.4f  (%.1f%% of roofline, %s)\n', ...
        k.name, k.ai, k.gflops, attainable, pct, region);
    scatter(k.ai, k.gflops, 70, markers{mod(i-1, numel(markers)) + 1}, 'filled', ...
        'DisplayName', strrep(k.name, '_', '\_'));
end

set(gca, 'XScale', 'log', 'YScale', 'log');
xlabel('Arithmetic Intensity (FLOP/byte)');
ylabel('Performance (GFLOP/s)');
title('Roofline Model — GH-Bench kernels (hand cross-check)');
legend('Location', 'southoutside', 'NumColumns', 2);
grid on;
hold off;

%% ---- Amdahl's-Law worked example -----------------------------------------
% speedup(p) = 1 / (f + (1-f)/p), f = serial fraction = phase1/(phase1+phase2)
figure('Name', 'Amdahl''s Law', 'Color', 'w');
hold on;
p_range = 1:8;
plot(p_range, p_range, 'k--', 'DisplayName', 'ideal (linear)');

for i = 1:numel(kernels)
    k = kernels(i);
    total = k.phase1_s + k.phase2_s;
    if total <= 0
        continue  % no phase timing reported for this kernel/row — skip
    end
    f = k.phase1_s / total;
    speedup = 1 ./ (f + (1 - f) ./ p_range);
    fprintf('%-28s serial fraction f=%.4f  predicted speedup @8 cores=%.3f\n', ...
        k.name, f, speedup(end));
    plot(p_range, speedup, '-o', 'DisplayName', ...
        sprintf('%s (f=%.3f)', strrep(k.name, '_', '\_'), f));
end

xlabel('Cores (p)');
ylabel('Predicted speedup');
title('Amdahl''s Law — predicted speedup by measured serial fraction');
legend('Location', 'southoutside', 'NumColumns', 2);
grid on;
hold off;
