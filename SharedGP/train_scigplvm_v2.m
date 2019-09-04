function model = train_scigplvm_v2(yTr,rank,kerType)
% share conditional independent gplvm
% yTr must be cell. each contain n x di matrix 
% logg:
% v2: using existing model log_evidence. 
rng(1)
a0 = 1e-3; b0 = 1e-3;

assert( iscell(yTr),'wrong yTr formate')

% U = rand(size(yTr{1},1),rank);
iInit = 1;
switch iInit 
    case 1 
        if size(yTr{1},2) >= rank
            [U,~,~] = svds(yTr{1},rank);
        else 
            [U,~,~] = svd(yTr{1});
            U = U(:,1:rank);
        end
            
    case 2 
        options.dim_new = rank;
        [U,~] = Isomaps(yTr{1},options);
    case 3 
        options.dim_new = rank;
        [U,~] = Isomaps_share(yTr,options);
end
        
params = U(:);
for i = 1:length(yTr)    %number of space
    
    log_bta{i} = log(1/var(yTr{i}(:)));
    log_sigma{i} = 0;
    log_sigma0{i} = log(1e-4);
    log_l{i} = zeros(rank,1);
    
    mShape{i} = size(yTr{i});
    
    D{i} = yTr{i}*yTr{i}';

    params = [params;log_l{i};log_sigma{i};log_sigma0{i};log_bta{i}];
end
N = mShape{1}(1);

%     params.U = U;
%     params.log_l = log_l;
%     params.log_bta = log_bta;
%     params.log_sigma = log_sigma;
%     params.log_sigma0 = log_sigma0;
%     params = [U(:);log_l;log_sigma;log_sigma0;log_bta];

%     fastDerivativeCheck(@(params) log_evidence_scigplvm(params,kerType, a0, b0,mShape, rank, D), params)
    fastDerivativeCheck(@(params) log_evidence_share(params,kerType, a0, b0, rank, yTr), params)
    
    
    %max_iter = 1000;
    %new_param = minimize(param, @(param) log_evidence_lower_bound(param, x, y, m), max_iter);
    opt = [];
    opt.MaxIter = 100;
    opt.MaxFunEvals = 10000;
%     new_params = minFunc(@(params) log_evidence_scigplvm(params,kerType, a0, b0,mShape, rank, D), params, opt);
    new_params = minFunc(@(params) log_evidence_share(params,kerType, a0, b0, rank, yTr), params, opt);
    params = new_params;
    fastDerivativeCheck(@(params) log_evidence_share(params,kerType, a0, b0, rank, yTr), params)
    
    U = reshape(params(1:N*rank), N, rank);
    idx = N * rank;
    for i = 1:length(D)
        [ker_params{i},idx] = load_kernel_parameter(params, rank, kerType, idx);    
        bta{i} = exp(params(idx+1));
        idx = idx+1;
        
        Sigma{i} = 1/bta{i}*eye(N) + ker_func(U,ker_params{i});
        Knn{i} = ker_cross(U, U, ker_params{i});
        train_pred{i} = Knn{i}*(Sigma{i}\yTr{i});
        
    end
    model = [];
    model.ker_params = ker_params;
    model.bta = bta;
    model.U = U;
    model.params = params;
    model.kerType = kerType;
    model.train_pred = train_pred;
    model.yTr = yTr;
    
end

function [f, df] = log_evidence_share(params,kerType, a0, b0, rank, yTr)

    [N,m] = size(yTr{1});
%     U = reshape(params(1:N*rank), N, rank);
    f = 0;
%     df = zeros(N*rank,1);
    df = zeros(size(params));
    
    idx = N*rank; 
    for i = 1:length(yTr)
        iparams = [params(1:N*rank); params(idx+1:idx+3+rank)];
        
        [f_i, df_i] = log_evidence_cigplvm(iparams,kerType, a0, b0, m, N, rank, yTr{i}*yTr{i}');
        
        f = f + f_i;
        df(1:N*rank) = df(1:N*rank) + df_i(1:N*rank);
        df(idx+1:idx+3+rank) = df_i(N*rank+1:end);
        
        idx = idx+3+rank;
    end

end