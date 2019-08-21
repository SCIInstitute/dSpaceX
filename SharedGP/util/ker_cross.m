%support ARD kernel only, k(x,y) = \sigma exp(-\frac{1}{2} (x-y)^\top diag(1/\l_k)) + \sigma0
function K = ker_cross(X, Y, ker_param)
    m = size(X,1);
    n = size(Y,1);
    if strcmp(ker_param.type, 'ard')
        sigma = ker_param.sigma;
        l = ker_param.l;
        X = X*diag(1./sqrt(l));
        Y = Y*diag(1./sqrt(l));
        P = sum(X.*X,2);
        Q=sum(Y.*Y,2);
        K = sigma*exp(-0.5*(repmat(P,1,n) + repmat(Q',m,1) ... 
             - 2*(X*Y')));
        K = K + (ker_param.sigma0+ker_param.jitter)*eye(m,n);
    elseif strcmp(ker_param.type, 'linear')
         K = X*Y';
    elseif strcmp(ker_param.type, 'ard-noSigma0')     
         sigma = ker_param.sigma;
        l = ker_param.l;
        X = X*diag(1./sqrt(l));
        Y = Y*diag(1./sqrt(l));
        P = sum(X.*X,2);
        Q=sum(Y.*Y,2);
        K = sigma*exp(-0.5*(repmat(P,1,n) + repmat(Q',m,1) ... 
             - 2*(X*Y')));
    else
        error('UnSupported kernel type');
    end
end

%cross kernel function between X and Y (n*k, m*k)
%return kenrel, n*m
% function kernel = ker_cross(X, Y, param)
% kernel = param;
% n = size(X, 1);
% m = size(Y, 1);
% 
% switch param.type
%     case 'linear'
%         kernel.K = X*Y';
%     case 'rbf'
%         P=sum(X.*X,2);
%         Q=sum(Y.*Y,2);
%         kernel.K = param.weight* exp(-param.gamma*(repmat(P,1,m) + repmat(Q',n,1) ... 
%             - 2*(X*Y')));
%     case 'ard'
%         X = X*diag(sqrt(param.gamma));
%         Y = Y*diag(sqrt(param.gamma));
%         P=sum(X.*X,2);
%         Q=sum(Y.*Y,2);
%         kernel.K = param.weight* exp(-(repmat(P,1,m) + repmat(Q',n,1) ... 
%             - 2*(X*Y')));
%         
%     case 'exp'
%         P=sum(X.*X,2);
%         Q=sum(Y.*Y,2);
%         kernel.K = param.weight* exp(-param.gamma*(repmat(P,1,m) + repmat(Q',n,1) ... 
%             - 2*(X*Y')).^0.5);
%     case 'poly'
%         kernel.K = (X*Y').^param.degree;    
%     otherwise
%         error('Unknown kernel type');
% end