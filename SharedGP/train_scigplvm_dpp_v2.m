function model = train_scigplvm_dpp_v2(yTr,rank,kerType)
% share conditional independent gplvm
% yTr must be cell. each contain n x di matrix 
% logg:
% v2: using existing model log_evidence. 
% dpp: add Dirichlet process prior
rng(1)
a0 = 1e-3; b0 = 1e-3;

assert( iscell(yTr),'wrong yTr formate')

% U = rand(size(yTr{1},1),rank);
% [U,~,~] = svds(yTr{1},rank);
%     %init isomap
%     options.dim_new=rank; 
%     [U,model] = Isomaps(yTr,options);

iInit = 1;
switch iInit 
    case 1 
        [U,~,~] = svds(yTr{1},rank);
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


    %% add DPM parameters
    nmod = 1;
    nvec = N;   %number of observations

    model.update_inv_cov = true;

    model.dp_lam = 1e1*ones(nmod,1);
    model.dim = rank(1);
    model.T = round(nvec/10);   
    model.dp_alpha = 1.0;
    model.dp_ga = cell(nmod,1);
    model.dp_logphi = cell(nmod,1);
    model.dp_lam_a0 = 1e-1*ones(1,nmod);
    model.dp_lam_b0 = 1e-3*ones(1,nmod);
    model.dp_lam_a = zeros(1, nmod);
    model.dp_lam_b = zeros(1,nmod);

    model.stat.ex_logv = cell(nmod,1);
    model.stat.dp_phi = cell(nmod,1);
    model.stat.eta_mean = cell(nmod,1);
    model.stat.eta_var = cell(nmod,1);
    model.stat.eta_cov = cell(nmod,1);
    for k=1:nmod
        trunc_no = model.T(k);
        model.dp_ga{k} = zeros(trunc_no,2);
        model.dp_logphi{k} = zeros(nvec(k),trunc_no);
        model.stat.ex_logv{k} = zeros(trunc_no,2);
        model.stat.eta_mean{k} = zeros(trunc_no,model.dim);
        model.stat.eta_var{k} = zeros(1,trunc_no);
        model.stat.eta_cov{k} = zeros(1,trunc_no);
    end
    %initilize with EM Gaussian mixture
    for k=1:nmod
        [~, model.stat.dp_phi{k}, gmm_model] = emgm(U',model.T(k));
        model.stat.eta_mean{k} = gmm_model.mu';
        model.stat.eta_cov{k} = ones(1, model.T(k));
        model.stat.eta_var{k} = model.dim + sum(model.stat.eta_mean{k}.^2,2)';
    end
%% main
    
    fastDerivativeCheck(@(params) log_evidence_share(params,kerType, a0, b0, rank, yTr,model), params)
    
    
    nIte = 40;
    opt = [];
    opt.MaxIter = 5;
    opt.MaxFunEvals = 10000;
    
    for i = 1:nIte
        U = reshape(params(1:N*rank), N, rank);
        
        model = E_step(model, {U}, 5);
        fastDerivativeCheck(@(params) log_evidence_share(params,kerType, a0, b0, rank, yTr,model), params)
        
        new_params = minFunc(@(params) log_evidence_share(params,kerType, a0, b0, rank, yTr,model), params, opt);
        params = new_params;

    end
    
%%
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
%     model = [];
    model.ker_params = ker_params;
    model.bta = bta;
    model.U = U;
    model.params = params;
    model.kerType = kerType;
    model.train_pred = train_pred;
    model.yTr = yTr;
    
end



function [f, df] = log_evidence_share(params,kerType, a0, b0, rank, yTr,model)

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

    %dpp
    idx = 0;
    U = reshape(params(idx+1:idx+N*rank), N, rank);
    
    k = 1;
    logL = -0.5*model.dp_lam(k)*  norm((U - model.stat.dp_phi{k}*model.stat.eta_mean{k}),'fro')^2;
    dU = -model.dp_lam(k)* (U - model.stat.dp_phi{k}*model.stat.eta_mean{k});
    
%     f_i = f_i - logL;
%     df_i(1:N*rank) = df_i(1:N*rank) - dU(:);
    
    f = f - logL;
    df(1:N*rank) = df(1:N*rank) - dU(:);
end


function model = E_step(model,U, nIte)

    %fix model
    nmod = 1;
%     nIte = 5;
%     U = model.U;
    nvec = size(U{1},1);

    for iter = 1:nIte
        %update q(eta), the cluster centers sampled from base measure
        for k=1:nmod    
            model.stat.eta_cov{k} = 1./(1 + model.dp_lam(k)*sum(model.stat.dp_phi{k},1));
            model.stat.eta_mean{k} = model.stat.dp_phi{k}'*U{k}./(1/model.dp_lam(k) + repmat(sum(model.stat.dp_phi{k},1)',1,size(U{k},2)) );
            model.stat.eta_var{k} = model.dim*model.stat.eta_cov{k} + sum(model.stat.eta_mean{k}.^2,2)';
        end

        %update q(Z), indicators sampled from stick-breaking process
        for k=1:nmod
            trunc_no = model.T(k);
            sum_t = zeros(1,trunc_no);
            ex_logv2 = model.stat.ex_logv{k}(:,2);
            for t=2:trunc_no
                sum_t(t) = sum_t(t-1) + ex_logv2(t-1);
            end
            for n=1:nvec(k)
                model.dp_logphi{k}(n,:) = model.stat.ex_logv{k}(:,1)' + sum_t - 0.5*model.dp_lam(k)*model.stat.eta_var{k} ...
                    + model.dp_lam(k)*U{k}(n,:)*model.stat.eta_mean{k}';
                model.stat.dp_phi{k}(n,:) = exp_sum_dist(model.dp_logphi{k}(n,:));
            end
        end

        %update q(V), for stick-breaking process construction
        for k=1:nmod
            trunc_no = model.T(k);
            model.dp_ga{k}(:,1) = 1 + sum(model.stat.dp_phi{k}, 1)';    
            for t=1:trunc_no
                model.dp_ga{k}(t,2) = model.dp_alpha + sum(sum(model.stat.dp_phi{k}(:,t+1:end), 1));
            end
            disum = psi(sum(model.dp_ga{k},2));
            model.stat.ex_logv{k} = [psi(model.dp_ga{k}(:,1)) - disum, psi(model.dp_ga{k}(:,2)) - disum];    
        end

        %update q(lam), for inverse variance of DP generating latent factors
        if model.update_inv_cov
            for k=1:nmod
                model.dp_lam_a(k) = model.dp_lam_a0(k) + 0.5*nvec(k)*model.dim;
                model.dp_lam_b(k) = model.dp_lam_b0(k) + 0.5*(trace(U{k}*U{k}') + sum(model.stat.dp_phi{k}*model.stat.eta_var{k}')) ...
                                    - sum(sum(model.stat.eta_mean{k}*U{k}'.*model.stat.dp_phi{k}'));
                model.dp_lam(k) = model.dp_lam_a(k)/model.dp_lam_b(k);                
            end
        end

    end
    
end

