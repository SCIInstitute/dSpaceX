%grad for tr(A \d Kmn), where Kmn = k(X, Y)
%support ARD kernel only, k(x,y) = \sigma exp(-\frac{1}{2} (x-y)^\top
%diag(1/\l_k)) + \sigma0
%ker_param.type
%ker_param.sigma0, .sigma, 
%ker_param.jitter
%return vec([dX, d_kernel_parameters])
function [g_X, g_ker_para] = ker_cross_grad(A, Kmn, X, Y, ker_param)
    if strcmp(ker_param.type, 'ard')
        [n,~] = size(X);
        l = ker_param.l;
        B = A'.*Kmn;
        s = sum(B,2);
        s2 = sum(B,1);
        Z = X*diag(1./sqrt(l));
        T = Y*diag(1./sqrt(l));
        dX = -diag(s)*X*diag(1./l) + B*Y*diag(1./l);
        d_log_l = vec(0.5*sum(Z.*(diag(s)*Z),1) + 0.5*sum(T.*(diag(s2)*T),1) - sum(Z.*(B*T), 1));
        d_log_sigma = sum(vec(B));
        %g = [vec(dX);d_log_l;d_log_sigma];
        g_X = dX;
        g_ker_para = [d_log_l;d_log_sigma;0];
    else
        error('UnSupported kernel type');
    end
end