% Read the data from CSV file.
raw_data = csvread('MMix.csv');

% Eliminate bad columns.
X_raw = raw_data(:, [2 3 4 5]);
y = raw_data(:, 1);

% Scale features.
function X_scaled = scaleFeatures(X_raw)
  num_examples = size(X_raw, 1);
  X_scaled = ...
    (X_raw - repmat(mean(X_raw), num_examples, 1)) ./ ...
    repmat(std(X_raw), num_examples, 1);
end
X_scaled = scaleFeatures(X_raw);

% Add higher order features.
function p = higherOrderPowers(a, k)
  p = zeros(size(a, 1), k + 1);
  for i = [0:k]
      p(:, i+1) = a.^i;
  end
end

function X = datasetWithHigherOrderfeatures(X_scaled)
  X1_pows = higherOrderPowers(X_scaled(:, 1), 4);
  X2_pows = higherOrderPowers(X_scaled(:, 2), 4);
  X3_pows = higherOrderPowers(X_scaled(:, 3), 4);
  X4_pows = higherOrderPowers(X_scaled(:, 4), 4);
  
  num_examples = size(X_scaled, 1);
  X = zeros(num_examples, 1);
  col = 1;
  for i = [0:4]
    for j = [0:4]
      for k = [0:4]
        for l = [0:4]
          X(:, col) = ...
            X1_pows(:, i+1) .* ...
            X2_pows(:, j+1) .* ...
            X3_pows(:, k+1) .* ...
            X4_pows(:, l+1);
          col += 1;
        end
      end  
    end
  end
end
X = datasetWithHigherOrderfeatures(X_scaled);

% Initialize regression coefficients with zeros.
coeff = zeros(size(X, 2), 1);

% Setup regularization coefficient.
reg_coeff = 1;

% Function to compute regularization cost.
function reg_cost = regularizationCost(coeff)
  reg_cost = sum(coeff' * coeff) - coeff(1) * coeff(1);
end

% Function to evaluate predictions.
function p = prediction(X, coeff)
  p = X * coeff;
end

% Function to evaluate cost and gradient.
function [cost, gradient] = computeCostAndGradient(X, y, coeff, reg_coeff)
  hypothesis = prediction(X, coeff);
  error = hypothesis - y;
  num_examples = length(y);
  cost = ...
    (sum(error' * error) + (reg_coeff * regularizationCost(coeff))) / ...
    (2 * num_examples);
  gradient = ((X' * error) + (reg_coeff * coeff)) / num_examples;    
end

# Run an unconstrained minimization.
options = optimset('GradObj', 'on', 'MaxIter', 400);
[final_coeff, final_cost] = ...
  fminunc(@(t)(computeCostAndGradient(X, y, t, reg_coeff)), coeff, options);

# Compare predicted versus actual values.
predicted_y = prediction(X, final_coeff);
error = 100 * (abs(predicted_y - y) ./ y);
plot(error, predicted_y, 'ro');

fprintf("Mean error: %f\n", mean(error));
fprintf("Stdev error: %f\n", std(error));