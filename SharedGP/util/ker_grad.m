%grad for tr(A \d Kmm), where Kmm = k(X, X)
%support ARD kernel only, k(x,y) = \sigma exp(-\frac{1}{2} (x-y)^\top
%diag(1/\l_k)) + \sigma0
%ker_param.type
%ker_param.sigma0, .sigma, 
%ker_param.jitter
%return vec([dX, d_log_kernel_parameters])
function [g_X, g_ker_para] = ker_grad(X, A, Kmm, ker_param)
    if strcmp(ker_param.type, 'ard')
        [n,~] = size(X);
        l = ker_param.l;
        B = A'.*(Kmm - (ker_param.sigma0 + ker_param.jitter)*eye(n));
        Z = X*diag(1./sqrt(l));
        ss = vec(sum(B,1)) + sum(B,2);
        dX = (B + B' - diag(ss) )*(X*diag(1./l));
        d_log_l = vec(0.5*sum(Z.*(diag(ss)*Z),1) - sum(Z.*(B*Z), 1));
        d_log_sigma = sum(vec(B));
        d_log_sigma0 = ker_param.sigma0*trace(A);
        g_X = dX;
        g_ker_para = [d_log_l;d_log_sigma;d_log_sigma0];
    elseif strcmp(ker_param.type, 'linear')
        g_X = 2*A'*X;
        d_log_sigma0 = ker_param.sigma0*trace(A);
        d_log_l = zeros(size(ker_param.l));
        d_log_sigma = zeros(1);
        g_ker_para = [d_log_l;d_log_sigma;d_log_sigma0];
    elseif strcmp(ker_param.type, 'ard-linear')
        [n,~] = size(X);
        l = ker_param.l;
        XX = X*X';
        Kmm = Kmm - ker_param.alpha*XX;
        B = A'.*(Kmm - (ker_param.sigma0 + ker_param.jitter)*eye(n));
        Z = X*diag(1./sqrt(l));
        ss = vec(sum(B,1)) + sum(B,2);
        dX = (B + B' - diag(ss) )*(X*diag(1./l)) + 2*ker_param.alpha*(A'*X);
        d_log_l = vec(0.5*sum(Z.*(diag(ss)*Z),1) - sum(Z.*(B*Z), 1));
        d_log_sigma = sum(vec(B));
        d_log_sigma0 = ker_param.sigma0*trace(A);
        d_log_alpha = ker_param.alpha*sum(vec(A.*XX));
        g_X = dX;
        g_ker_para = [d_log_l;d_log_sigma;d_log_sigma0;d_log_alpha];
    elseif strcmp(ker_param.type, 'ard-noSigma0')    
        [n,~] = size(X);
        l = ker_param.l;
        B = A'.*(Kmm - (ker_param.sigma0 + ker_param.jitter)*eye(n));
        Z = X*diag(1./sqrt(l));
        ss = vec(sum(B,1)) + sum(B,2);
        dX = (B + B' - diag(ss) )*(X*diag(1./l));
        d_log_l = vec(0.5*sum(Z.*(diag(ss)*Z),1) - sum(Z.*(B*Z), 1));
        d_log_sigma = sum(vec(B));
%         d_log_sigma0 = ker_param.sigma0*trace(A);
        d_log_sigma0=0;
        g_X = dX;
        g_ker_para = [d_log_l;d_log_sigma;d_log_sigma0];
    else
         error('UnSupported kernel type');
    end
    
end 

