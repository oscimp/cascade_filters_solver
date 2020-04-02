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

mkdir firls;

gabarit = [0 0.45 0.55 1];
point = [1 1 0 0];
numberCoeff = [3:2:60];
numberBit = [2:22];
fc = 0.5;
N = 2048;

% For all coefficients
for it_coeff = 1:length(numberCoeff)
    n_coeff = numberCoeff(it_coeff);

    % Generate the float coefficients
    b = firls(n_coeff - 2, gabarit, point);

    % For all bit
    for it_nob = 1:length(numberBit)
        nob = numberBit(it_nob);

        h = b/max(b); % Normalize
        b_int = round(h*(power(2,nob-1)-1)); % Scale to max int

        % Write coeffiecent file
        filename = sprintf("firls/firls_%03d_int%02d", n_coeff, nob);
        f = fopen(filename, "w");
        fprintf(f, "%i\n", b_int);
        fclose(f);

    endfor
endfor

mkdir fir1;

numberCoeff = [3:60];
numberBit = [2:18];
fc = 0.5;
N = 2048;

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

        % Write coeffiecent file
        filename = sprintf("fir1/fir1_%03d_int%02d", n_coeff, nob);
        f = fopen(filename, "w");
        fprintf(f, "%i\n", b_int);
        fclose(f);

    endfor
endfor
