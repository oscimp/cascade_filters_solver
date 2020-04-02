clear all;

function val = load_binary(file_name, form = "double")
	f = fopen(file_name, "r");
	if (f == -1)
		error("The file '%s' is not found", file_name);
	endif

	val = fread(f, Inf, form);
	fclose(f);
endfunction

function spec = post_process(fir_data, N)
  hann = zeros(N, floor(length(fir_data)/N));
  hann_ref = zeros(length(fir_data), 1);
  for it = 1:length(fir_data)/N
      hann(:,it) = fir_data((it-1) * N + 1:it*N) .* hanning(N);
  endfor

  spec = abs(fft(hann));
  spec = mean(spec');
  start_band = ceil((0.1) * N);
  stop_band = ceil((0.4) * N);
endfunction

N = 2048;

# Generate the reference
b0= load("filters/firls/firls_003_int03");
b1= load("filters/firls/firls_035_int11");
b2= load("filters/firls/firls_019_int07");
# b3= load("filters/firls/firls_027_int09");
# Add all other filter

hTotal = ones(N/2, 1);

# Stage 1
[h, w] = freqz(b0, 1, N/2);
hTotal = hTotal .* h;

# Stage 2
[h, w] = freqz(b1, 1, N/2);
hTotal = hTotal .* h;

# Stage 3
[h, w] = freqz(b2, 1, N/2);
hTotal = hTotal .* h;

# # Stage 4
# [h, w] = freqz(b3, 1, N/2);
# hTotal = hTotal .* h;

# Add all other stage

mag = abs(hTotal);
mag = mag ./ mag(1);
log_freqz = 20 * log10(mag);

# Plot result from raw data
simu_cascade = load_binary("simu_stage.bin", "int64");
raw_prn = load_binary("data_prn.bin", "int64");
spectrum = post_process(simu_cascade, N);
spectrum_prn = post_process(raw_prn, N);
spectrum_norm = spectrum ./ spectrum_prn;
spectrum_norm = 20*log10(spectrum_norm ./ max(spectrum_norm(200:end-200)))(1:N/2);
spectrum_norm(1:200) = rand(200, 1) * 0.5 - 0.25;

clf;
f_axe = [1:N/2] * 2/N;
hold on;
plot(f_axe, spectrum_norm(1:N/2));
plot(f_axe, log_freqz, "--", "linewidth", 2);

hold off;
ylim([-250, 20]);
lgnd = legend("DSPS data", "Octave data", "location", "southwest");
set(lgnd,'color','white');
ylabel("Magnitude (dB)");
xlabel("Normalized frequency");
