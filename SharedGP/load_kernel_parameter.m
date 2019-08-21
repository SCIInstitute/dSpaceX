%assemble a kenel parameter obj. from parameter list
function [ker_param, idx] = load_kernel_parameter(param_vec, d, type, start_idx)
    l = exp(param_vec(start_idx+1:start_idx+d));
    sigma = exp(param_vec(start_idx+d+1));
    sigma0 = exp(param_vec(start_idx+d+2));
    ker_param = [];
    ker_param.type = type;
    if strcmp(type, 'ard')
        ker_param.l = l;
        ker_param.sigma = sigma;
        ker_param.sigma0 = sigma0;
        ker_param.jitter = 1e-1;
        ker_param.jitter = 1e-8;
        %ker_param.jitter = 1e-1;
%         ker_param.jitter = 1e-4;
        %ker_param.jitter = 1e-1;
        %ker_param.jitter = 1e-3;
        idx = start_idx + d + 2;
    elseif strcmp(type, 'linear')
        ker_param.l = l;
        ker_param.sigma = sigma;
        ker_param.sigma0 = sigma0;
        ker_param.jitter = 1e-6;
         idx = start_idx + d + 2;
    elseif strcmp(type, 'ard-linear')
        ker_param.alpha = exp(param_vec(start_idx+d+3));    
        ker_param.l = l;
        ker_param.sigma = sigma;
        ker_param.sigma0 = sigma0;
        ker_param.jitter = 1e-1;
        idx = start_idx + d + 3; 
    elseif strcmp(type, 'ard-noSigma0')
        ker_param.l = l;
        ker_param.sigma = sigma;
%         ker_param.sigma0 = sigma0;
        ker_param.sigma0 = 0;
        ker_param.jitter = 1e-1;
        %ker_param.jitter = 1e-10;
        %ker_param.jitter = 1e-1;
        %ker_param.jitter = 1e-4;
        %ker_param.jitter = 1e-1;
        %ker_param.jitter = 1e-3;
        idx = start_idx + d + 2;
    end
   
end