% Isomaps_PreImage_Demo
% Demonstration of PreImage solver on Isomaps
% 
% Instructions:
% 1) Change the parameter section to see performance of Isomaps_PreImage.
% Tips: 
% Less neighbor points (by decrease options.neighborPara) but enough
% for the Isomap to carry out(Graph is connected) promise a better
% reconstruction!
%
% Modifications:
% WeiX, lost date,    first edition 
% WeiX, Dec-4th-2014, Add comment

%% Initialization
clear
% close all

%% parameters
Num_data=400;
dim_new=2;
              
% Isomap options
options.dim_new=dim_new;                      % New dimension
options.neighborType='k';               % Type of neighbor.Choice:1)'k';Choice:2)'epsilon'
options.neighborPara=5;                % parameter for choosing the neighbor. number of neighbor for "k" type and radius for 'epsilon'
options.metric='euclidean';             % Method of measurement. Metric

% Isomap PreImage options
preoptions.neighborType='k';    % Type of neighbor of new point. Choice:1)'k';Choice:2)'epsilon'
preoptions.neighborPara=5;     % Parameter of neighbor of new point
% preoptions.type='Dw';           % Type of distance to coordinate method. Distance weight/Least square estimate
% preoptions.para=5;              % Parameter of distance to coordinate recover method

preoptions.type='Exp';  
preoptions.neighbor=5;
% preoptions.type='LpcaI';
% preoptions.dim_new=3; % Use to stable the result but sacrefy accuracy

%% Data Generation Swiss Roll Dataset
% Num_data=Num_train+Num_test;
t1=rand(Num_data,1)*4*pi;   % Theater
t2=rand(Num_data,1)*20;
t1=sort(t1);                
X(:,1)=t1.*cos(t1);         % X
X(:,2)=t2;                  % Y
X(:,3)= t1.*sin(t1);        % Z
color = t1;                     % Color according to Theater
size = ones(Num_data,1)*10;    % Size. Constant

% % Take out train & test dataset
% test_index=round(rand(Num_test,1)*Num_data);
% test_index = randperm(Num_data,Num_test);
% X_starOrigi=X(test_index,:);
% X(test_index,:)=[];


%% Main
[Z,model] = Isomaps(X,options);
% X_star = Isomaps_PreImage(Z,model,options);

for index =1:Num_data
    Z_star=Z(index,:);
    model_temp=model;       % model_temp is model without the index point to avoide 100% accurate recovery.
    model_temp.Z=removerows(Z,'ind',index);  
    model_temp.X=removerows(X,'ind',index);  
    X_star(index,:) = Isomaps_PreImage(Z_star,model_temp,preoptions);
end

%% Ploting
figure(1)
scatter3(X(:,1),X(:,2),X(:,3),size,color);
title('Original dataset')

figure(2)
scatter(Z(:,1),Z(:,2),size,color);
title(sprintf('Projection with %d principal components',dim_new))

figure(3)
scatter3(X_star(:,1),X_star(:,2),X_star(:,3),size,color);
title(sprintf('Reconstruction of original dataset with %d principal components',dim_new))

figure(4)
plot(real(model.eigenvalues(1:5)),'k'); %5 migh be change according to different data.
hold on 
plot(model.eigenvalues(1:dim_new),'r');
hold off
title(sprintf('Energy Decay'))

figure(5)
plot(X_star,'b--')
hold on 
plot(X,'r-')
hold off
title('Prediction Vs Real Curves')



