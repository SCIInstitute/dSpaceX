function model = sgplvm_invGp_v1(model,k,yi)
% using GP to model the inverse of share gplvm model
% model: trained model. must include the latent U.
% k: the k-th output observation indicator.
% yi: the new observed value for k-th output

%% train a GP from output to input (using cigp)
% invGPi = train_cigp_v2(model.train_pred{i}, model.U, yi);
invGPi = train_cigp_v2(model.yTr{k}, model.U, yi);

Unew = invGPi.pred_mean;

%% updating/ predicting the corresponding other output
% yTr = model.train_pred;
yTr = model.yTr;

params = model.params;
kerType = model.kerType;


[N,m] = size(yTr{1});
rank = size(model.U,2);

U = reshape(params(1:N*rank), N, rank);
idx = N * rank;
% iU = Unew;

for i = 1:length(yTr)
    [ker_params{i},idx] = load_kernel_parameter(params, rank, kerType, idx);    
    bta{i} = exp(params(idx+1));
    idx = idx+1;
    [iN,im] = size(yTr{i});
    
    iU = U(1:iN,:);

    if isfield(model, 'ifProbit')
        if model.ifProbit{i}
%             Sigma{i} = 1*eye(iN) + ker_func(iU,ker_params{i});
    %         Sigma{i} = ker_func(iU,ker_params{i});
            Sigma{i} =  1*eye(iN) + ker_func(iU,ker_params{i});        
            Kmn = ker_cross(Unew, iU, ker_params{i});
%             Knn{i} = ker_cross(Unew, iU, ker_params{i});
%             yNew{i} = Knn{i}*(Sigma{i}\model.H{i});
            Kmm = ker_func(Unew,ker_params{i});
            
            yNew{i} = Kmn*(Sigma{i}\model.H{i});

            % integrating out variance 
%             varr = Kmm - Kmn*(Sigma{i} \ Kmn');
%             varr = diag(varr);
%             varr = repmat(varr,1,size(yNew{i},2));    
            varr=0;

            yNew{i} = normcdf(yNew{i}./sqrt(1+varr));

        else
            Sigma{i} = 1/bta{i}*eye(iN) + ker_func(iU,ker_params{i});
            Knn{i} = ker_cross(Unew, iU, ker_params{i});
            yNew{i} = Knn{i}*(Sigma{i}\yTr{i});
        end
    else 
        Sigma{i} = 1/bta{i}*eye(iN) + ker_func(iU,ker_params{i});
    %     Knn{i} = ker_cross(iU, Unew, ker_params{i});
        Knn{i} = ker_cross(Unew, iU, ker_params{i});
        yNew{i} = Knn{i}*(Sigma{i}\yTr{i});  
    end
    
%     model.uNew = Unew;
%     model.yNew = yNew;
    
    model.y_star = yNew;
    model.u_star = Unew;
    
end


end