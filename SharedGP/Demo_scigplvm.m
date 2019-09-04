%Demo_scigplvm

addpath(genpath('../code'));

clear

%% data generate
nTr = 50;
nTe = 100;
% nClass = 5;

% xTr = rand(nTr,1)*10;
% xTe = rand(nTe,1)*10;

zTr = rand(nTr,1)*10;
zTe = linspace(0.01,10,nTe)';

xTr = zTr + randn(nTr,1)*1;
xTe = zTe;

yTr = [cos(zTr).*zTr, sin(zTr).*zTr, rand(nTr,1)*1];   %swiss roll 
figure(1)
scatter3(cos(zTr).*zTr, sin(zTr).*zTr, rand(nTr,1),10,zTr);

yTe = [cos(zTe).*zTe, sin(zTe).*zTe, rand(nTe,1)*1];   %swiss roll 
%% use yTr to predict  xTr
% model1 = train_cigplvm(yTr,2,'ard');
Y{1} = yTr;
Y{2} = zTr;
% model1 = train_scigplvm_dpp_v2(Y,3,'ard');
model1 = train_scigplvm_v2(Y,10,'ard');

model2 = sgplvm_invGp_v1(model1,1,yTe);

model3 = train_scigplvm_dpp_infere_v2(model1,1,yTe);
% model3 = train_scigplvm_dpp_infere_v4(model1,1,yTe);  %for dpp model

model4 = scigplvm_infere_v3(model1,1,yTe);

%%
figure(1)
plot(zTe,model2.y_star{1},'r-')
hold on 
plot(zTe,model3.y_star{1},'b-')
plot(zTe,model4.y_star{1},'g-')
plot(zTe,yTe,'k-')
plot(zTr,yTr,'k+')
hold off

figure(2)
% plot(model2.y_star{2},'r-')
% hold on 
% plot(model3.y_star{2},'b-')
% plot(model4.y_star{2},'g-')
% plot(xTe,'k-')
% plot(zTr,'k+')
% hold off

plot(zTe,model2.y_star{2},'r-')
hold on 
plot(zTe,model3.y_star{2},'b-')
plot(zTe,model4.y_star{2},'g-')
plot(zTe,zTe,'k-')
plot(zTr,zTr,'k+')
hold off