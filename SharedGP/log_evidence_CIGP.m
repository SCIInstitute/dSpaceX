%log model evidence of CIGP: given the kernel and noise paramters, the GP
%projection is indepedent for each output
%X: N by d, d is input dimension
%D: YY\top, Y is N by m, m is the # of ouptut
function [f, df] = log_evidence_CIGP(params, a0, b0, m, X, D,kerType)
    [N,d] = size(X);
    [ker_params,idx] = load_kernel_parameter(params, d, kerType, 0);
    bta = exp(params(idx+1));
    K = ker_func(X, ker_params);
    Sigma = K + 1/bta*eye(N);    
    InvSigma = Sigma^(-1);
    InvSigmaD = Sigma\D;
    logL = (a0 - 1)*log(bta) - b0*bta - 0.5*m*logdet(Sigma) - 0.5*trace(InvSigmaD);
    dbta = (a0 - 1)/bta - b0 + 0.5*m*bta^(-2)*trace(InvSigma) - 0.5*bta^(-2)*sum(vec(InvSigmaD.*InvSigma));
    d_logbta = dbta*bta;
    A = -0.5*m*InvSigma + 0.5*InvSigmaD*InvSigma;
    [~, d_ker_params] = ker_grad(X, A, K, ker_params);
    df = [d_ker_params;d_logbta];
    %regularize kernel paramters, avoid going crazy
    logL = logL - 0.5*sum(params(1:end-1).*params(1:end-1));
    df(1:end-1) = df(1:end-1) - params(1:end-1);
    %turn it into minimization problem
    f = -logL;
    df = -df;
end