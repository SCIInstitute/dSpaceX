function model = train_scigplvm_infere_v3(model,k,yi_star)
% share conditional independent gplvm
% yTr must be cell. each contain n x di matrix 
% logg:
% v2: using existing model log_evidence.
% v3: an updated version (for cross validation)

rng(1)
a0 = 1e-3; b0 = 1e-3;
N = size(model.U,1);
rank = size(model.U,2);
N_star = size(yi_star,1);
yTr = model.yTr;
kerType = model.kerType;

assert( size(yi_star,2)==size(model.yTr{k},2) ,'not consistent yi_star with model')

% U = rand(size(yTr{1},1),rank);
% [U,~,~] = svds(yTr{1},rank);
%     %init isomap
%     options.dim_new=rank; 
%     [U,model] = Isomaps(yTr,options);

invGPi = train_cigp_v2(model.yTr{k}, model.U, yi_star);
U_star = invGPi.pred_mean;

N_all = N + N_star;

U = reshape(model.params(1:N*rank), N, rank);

U_all = [U;U_star];

yTr{k} = [yTr{k};yi_star];
% 
% 
% idx = N * rank;
% for i = 1:length(D)
%     [ker_params{i},idx] = load_kernel_parameter(params, rank, kerType, idx);    
%     bta{i} = exp(params(idx+1));
% end

% model.stat.dp_phi{1}=[model.stat.dp_phi{1};rand(N_star,model.T)];

params = U_all(:);
params_length(1) = length(params);
for i = 1:length(yTr)    %number of space
    
%     log_bta{i} = log(1/var(yTr{i}(:)));
%     log_sigma{i} = 0;
%     log_sigma0{i} = log(1e-4);
%     log_l{i} = zeros(rank,1);
%     
%     mShape{i} = size(yTr{i});
    
%     D{i} = yTr{i}*yTr{i}';

%     params = [params;log_l{i};log_sigma{i};log_sigma0{i};log_bta{i}];
    params = [params;log(model.ker_params{i}.l);log(model.ker_params{i}.sigma);log(model.ker_params{i}.sigma0);log(model.bta{i})];   
    params_length(i+1) = 3+rank;
end
%% main

%     fastDerivativeCheck(@(params) log_evidence_share_more(params,kerType, a0, b0, rank, yTr, N_all,model), params)
    fastDerivativeCheck(@(params)  log_evidence_share_miss(params,params_length, a0, b0, yTr,rank,N_all), params)
 
%     nIte = 100;
    opt = [];
    opt.MaxIter = 1000;
    opt.MaxFunEvals = 10000;
    
    new_params = minFunc(@(params) log_evidence_share_miss(params,params_length, a0, b0, yTr,rank,N_all), params, opt);
    params = new_params;
        
    fastDerivativeCheck(@(params)  log_evidence_share_miss(params,params_length, a0, b0, yTr,rank,N_all), params)
%%
    U_all = reshape(params(1:N_all*rank), N_all, rank);
    U_star = U_all(N+1:N+N_star,:);
    U = U_all(1:N, :);
    
    idx = N_all * rank;
    for i = 1:length(yTr)
        [ker_params{i},idx] = load_kernel_parameter(params, rank, kerType, idx);    
        bta{i} = exp(params(idx+1));
        idx = idx+1;
        
        Sigma{i} = 1/bta{i}*eye(N) + ker_func(U,ker_params{i});
        Knn{i} = ker_cross(U_star, U, ker_params{i});
        y_star{i} = Knn{i}*(Sigma{i}\yTr{i}(1:N,:));
        
    end
    
%     model = [];
    model.ker_params = ker_params;
    model.bta = bta;
    model.U = U;
    model.params = params;
    model.kerType = kerType;
%     model.train_pred = y_pred;
    model.yTr = yTr;
    model.y_star = y_star;
    model.u_star = U_star;
    
end

%%
function [f, df] = log_evidence_share_miss(params,params_length, a0, b0, yTr, dim_latent,N_total)
% likelihood for missing value 
    
    paraCell = vec2sPara(params,params_length);
    U = reshape(paraCell{1},N_total,dim_latent);
    
    df_u = zeros(size(paraCell{1}));
    f = 0;
    for i = 1:length(yTr)
        [iN,iDim] = size(yTr{i});
%         idx_use = idUse(params_length(1),1:iN,dim_latent);
        idx_use = idx_in_matrix(1:iN,N_total,dim_latent);
        
        params_i = [paraCell{1}(idx_use); paraCell{i+1}];
        [f_i, df_i] = log_evidence_cigplvm(params_i,'ard', a0, b0, iDim, iN, dim_latent, yTr{i}*yTr{i}');
        df_i_u = df_i(1:iN*dim_latent);
        df_i_hyp{i} = df_i(iN*dim_latent+1:end);
        
        df_u(idx_use) = df_u(idx_use) + df_i_u;
        f = f + f_i;
    end
    df = df_u;
    for i = 1:length(yTr)
        df = [df;df_i_hyp{i}];
    end

end
    
function [para_vec, para_length] = paraCell2vec(paraCell)
%     mpara is a cell
    para_vec = [];   
    for k = 1:length(paraCell)
        para_vec = [para_vec; paraCell{k}];
        para_length(k) = length(paraCell{k}); 
    end
end

function paraCell = vec2sPara(para_vec,para_length)

    for k = 1:length(para_length)
        paraCell{k} = para_vec(1:para_length(k));
        para_vec(1:para_length(k)) = [];
    end
end

function idx_used = uParaIdx(uParas,id_use,N,dim)
% u parameter vector used id 
% uParas is (N*d) matrix
% id_use is used Id, e.g., 1:N-1;
% idx_used gives the index 
    id = 1:length(uParas);
    idxm = reshape(id,N,dim);
    idxm = idxm(id_use,:);
    idx_used = idxm(:);
end

function idx = idx_in_matrix(col_id,N,dim)
% return index for a N*dim matrix using collum id id_use
    id = 1:N*dim;
    idxm = reshape(id,N,dim);
    idxm = idxm(col_id,:);
    idx = idxm(:);
end



