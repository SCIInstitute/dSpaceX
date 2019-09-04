%Demostration of share-gplvm across multiple data space.
% NOTE: initilization of the latent could matter. Now provide
% PCA/Isomap/share-isomap initilization. default is share-isomap. Change
% can be made in function:train_scigplvm_v2

clear 

addpath(genpath('./L-BFGS-B-C'))
addpath(genpath('./lightspeed'))
addpath(genpath('./minFunc_2012'))
addpath(genpath('./util'))
addpath(genpath('./ISOMAPS'))
%% prepare data
load('xs.mat')

IMG_SIZE1 = 60;
IMG_SIZE2 = 60;

y4 = design_parameters';
y1 = vmis_stress';
y2 = xs';
y3 = compliance';

ifUse = logical(mod(y4(:,1),2)) & y4(:,1)<10 & y4(:,1)>-10;  %reduce data size
y4 = y4(ifUse,:);
y1 = y1(ifUse,:);
y2 = y2(ifUse,:);
y3 = y3(ifUse,:);

nTr = 20;
nTe = 100;
idAll = randperm(size(y4,1));

idTr = idAll(1:nTr);
idTe = idAll(size(y4,1):-1:size(y4,1)+1-nTe);

Y{1} = y1(idTr,:);
Y{2} = y2(idTr,:);
Y{3} = y3(idTr,:);
Y{4} = y4(idTr,:);

%% train model with training data
rank = 5; %latent dimension
model = train_scigplvm_v2(Y,rank,'ard');

%% predicting all y and U (latent) given y1 (fast approach)
model2 = sgplvm_invGp_v1(model,1,y1(idTe,:));
model2.u_star
model2.y_star

%% predicting all y and U (latent) given y2. (improved approach)
% model3 = train_scigplvm_dpp_infere_v2(model,2,y2(idTe,:));
% model3 = train_scigplvm_infere_v2(model,2,y2(idTe,:));
% model3 = train_scigplvm_infere_v3(model,2,y2(idTe,:));
% model3 = sgplvm_invGp_v1(model,2,y2(idTe,:));
model3 = scigplvm_infere_v3(model,2,y2(idTe,:));
model3.u_star;
model3.y_star;

%% predicting all y given U (latent)
model4 = sgplvm_pred(model,model3.u_star);
model4.yNew 

%%
rank = 5; %latent dimension
model_dpp = train_scigplvm_dpp_v2(Y,rank,'ard');
model_dpp2 = train_scigplvm_dpp_infere_v4(model_dpp,2,y2(idTe,:));

