%log model evidence of CIGP: given the kernel and noise paramters, the GP
%projection is indepedent for each output
%X: N by d, d is input dimension
%D: YY\top, Y is N by m, m is the # of ouptut
function [f, df] = log_evidence_cigplvm(params,kerType, a0, b0, m, N, rank, D)

    %m dimention of y
    %N number of y
    %rank latent dimension
    %D = Y*Y';

    %% get model parameter
    idx = 0;
    U = reshape(params(idx+1:idx+N*rank), N, rank);
    [ker_params,idx] = load_kernel_parameter(params, rank, kerType, idx+N*rank);    
    bta = exp(params(idx+1));
    
    %% main
    K = ker_func(U, ker_params);
    Sigma = K + 1/bta*eye(N);    
    InvSigma = Sigma^(-1);
    InvSigmaD = Sigma\D;
    
    logL = (a0 - 1)*log(bta) - b0*bta - 0.5*m*logdet(Sigma) - 0.5*trace(InvSigmaD);
    dbta = (a0 - 1)/bta - b0 + 0.5*m*bta^(-2)*trace(InvSigma) - 0.5*bta^(-2)*sum(vec(InvSigmaD.*InvSigma));
    d_logbta = dbta*bta;
    A = -0.5*m*InvSigma + 0.5*InvSigmaD*InvSigma;
    [dU, d_ker_params] = ker_grad(U, A, K, ker_params);
    
    
    df = [dU(:);d_ker_params;d_logbta];
    
    %regularize kernel paramters, avoid going crazy
%     logL = logL - 0.5*sum(params(1:end-1).*params(1:end-1));
%     df(1:end-1) = df(1:end-1) - params(1:end-1);
    %turn it into minimization problem
    f = -logL;
    df = -df;
end