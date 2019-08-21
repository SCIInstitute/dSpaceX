%Demo_scigplvm

addpath(genpath('../code'));

clear

%% data generate
nTr = 100;
nTe = 100;
% nClass = 5;

% xTr = rand(nTr,1)*10;
% xTe = rand(nTe,1)*10;

xTr = linspace(0.01,10,nTr)';
xTe = linspace(0.01,10,nTe)'+randn(nTe,1)*.1;

yTr = [cos(xTr).*xTr, sin(xTr).*xTr, rand(nTr,1)];   %swiss roll 
figure(1)
scatter3(cos(xTr).*xTr, sin(xTr).*xTr, rand(nTr,1),10,xTr);

yTe = [cos(xTe).*xTe, sin(xTe).*xTe, rand(nTe,1)];   %swiss roll 
%%
% model1 = train_cigplvm(yTr,2,'ard');
Y{1} = yTr;
Y{2} = xTr;
model1 = train_scigplvm_dpp_v2(Y,3,'ard');

model2 = sgplvm_invGp_v1(model1,1,yTe);

model3 = train_scigplvm_dpp_infere_v2(model1,1,yTe);

model4 = train_scigplvm_infere_v2(model1,1,yTe);
%%
figure(1)
plot(model2.yNew{1},'r')
hold on 
plot(model3.y_star{1},'b')
plot(model4.y_star{1},'g.')
plot(yTe,'k')
hold off

figure(2)
plot(model2.yNew{2},'r')
hold on 
plot(model3.y_star{2},'b')
plot(model4.y_star{2},'g.')
plot(xTe,'k')
hold off