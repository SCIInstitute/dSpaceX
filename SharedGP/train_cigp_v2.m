function model = train_cigp_v2(xTr, yTr, xTe)
%train cigp model

    a0 = 1e-3; b0 = 1e-3;
    [N,d] = size(xTr);

    m = size(yTr,2);
    D = yTr*yTr';
    assert(size(xTr,1)==size(yTr,1),'inconsistent data');
    log_bta = log(1/var(yTr(:)));
    log_l = zeros(d,1);
    %log_l = 2*log(median(pdist(Xtr)))*ones(d,1);
    log_sigma = 0;
    log_sigma0 = log(1e-4);
    params = [log_l;log_sigma;log_sigma0;log_bta];
    fastDerivativeCheck(@(params) log_evidence_CIGP(params,a0,b0,m, xTr, D, 'ard'), params);
    %max_iter = 1000;
    %new_param = minimize(param, @(param) log_evidence_lower_bound(param, x, y, m), max_iter);
    opt = [];
    opt.MaxIter = 100;
    opt.MaxFunEvals = 10000;
    new_params = minFunc(@(params) log_evidence_CIGP(params,a0,b0,m, xTr, D, 'ard'), params,opt);
    [ker_param,idx] = load_kernel_parameter(new_params, d, 'ard', 0);
    bta = exp(new_params(idx+1));
    model = [];
    model.ker_param = ker_param;
    model.bta = bta;
    
    %test
    Sigma = 1/bta*eye(N) + ker_func(xTr,ker_param);
    Knn = ker_cross(xTr, xTr, ker_param);
    train_pred = Knn*(Sigma\yTr);

    model.ker_param = ker_param;
    model.train_pred = train_pred;
    model.bta = bta;    
    
    if ~isempty(xTe)
        Ksn = ker_cross(xTe,xTr,ker_param);
        pred_mean = Ksn*(Sigma\yTr);
        model.pred_mean = pred_mean;
    end
    
end

