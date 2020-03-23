clear all;

function rejection = rejection_criterion(log_data, fc)
    N = length(log_data);

    % Index of the first point in the tail fir
    index_tail = round((fc + 0.1) * N) + 1;

    % Index of th last point on the band
    index_band = round((fc - 0.1) * N);

    % Get the worst rejection in stopband
    worst_rejection = -max(log_data(index_tail:end));

    % Get the total of deviation in passband
    worst_band = -(max(log_data(1:index_band)) - min(log_data(1:index_band)));

    % Compute the rejection
    rejection = worst_band + worst_rejection;
endfunction

numberCoeff = [3:2:60];
numberBit = [2:22];
fc = 0.5;
N = 2048;

rejectionMatrix = zeros(length(numberCoeff), length(numberBit));
piOutMatrix = zeros(length(numberCoeff), length(numberBit));

% For all coefficients
for it_coeff = 1:length(numberCoeff)
    n_coeff = numberCoeff(it_coeff);

    % Generate the float coefficients
    b = fir1(n_coeff - 1, fc);

    % For all bit
    for it_nob = 1:length(numberBit)
        nob = numberBit(it_nob);

        h = b/max(b); % Normalize
        b_int = round(h*(power(2,nob-1)-1)); % Scale to max int

        % Compute the freqz
        [h, w] = freqz(b_int, 1, N);
        mag = abs(h);

        % Pass to log scale
        mag_dat = mag / mag(1);
        log_dat = 20 * log10(mag_dat);

        % Compute the rejection_critera
        rejection = rejection_criterion(log_dat, fc);

        % Store results
        rejectionMatrix(it_coeff,it_nob) = rejection;
    endfor
endfor

% Create coefficient file
f = fopen("../fir_data/firls_2_22_bits_3_2_60_coeffs.bin", "w");
for index_nob = 1:length(numberBit)
    for index_coeff = 1:length(numberCoeff)
        fwrite(f, numberBit(index_nob), "uint16");
        fwrite(f, numberCoeff(index_coeff), "uint16");
        fwrite(f, rejectionMatrix(index_coeff, index_nob), "double");
    endfor
endfor
fclose(f);
