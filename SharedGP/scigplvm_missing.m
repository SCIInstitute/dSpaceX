function scigplvm_missing(Ytr,rank,kerType)
% shared conditional GP with missing values and predic the missing value

assert( iscell(Ytr),'wrong yTr formate')
rng(1)
a0 = 1e-3; b0 = 1e-3;   %prior for noise 

[N,dim] = cellfun(@size,Ytr);
[N_max,k_max] = max(N);     %k_max is the yTr that has no missing value

iInit = 1;
switch iInit 
    case 1 
        [U,~,~] = svds(Ytr{1},rank);
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




end