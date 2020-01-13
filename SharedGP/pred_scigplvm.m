function [y_mean, y_cov] = pred_scigplvm(model, U_star)


params = model.params;
yTr = model.yTr;
rank = size(model.U,2);

N = size(yTr{1},1);

U = reshape(params(1:N*rank), N, rank);
idx = N * rank;
    
for i = 1:length(yTr)
    [ker_params{i},idx] = load_kernel_parameter(params, rank, model.kerType, idx);    
    bta{i} = exp(params(idx+1));
    idx = idx+1;
    [iN,im] = size(yTr{i});

%         Sigma{i} = 1/bta{i}*eye(N) + ker_func(U,ker_params{i});
%         Knn{i} = ker_cross(U, U, ker_params{i});
%         train_pred{i} = Knn{i}*(Sigma{i}\yTr{i});

    iU = U(1:iN,:);
    Sigma{i} = 1/bta{i}*eye(iN) + ker_func(iU,ker_params{i});
    Knn{i} = ker_cross(U_star, iU, ker_params{i});
    y_mean{i} = Knn{i}*(Sigma{i}\yTr{i});
    
    y_cov{i} = diag(ker_cross(U_star, U_star, ker_params{i})) - diag(Knn{i}*(Sigma{i}\Knn{i}'));
%     y_cov{i} = ker_cross(U_star, U_star, ker_params{i}) - Knn{i}*inv(Sigma{i})*Knn{i}';
    
    
end


end