% Kpca_Demo
% Demonstration of KPCA
%
% Modifications:
% WeiX, Dec-1th-2014, first edition 

clear
%% Parameters 
Num_data=200;
new_dim=4;

% Isomap options
options.dim_new=new_dim;                      % New dimension
options.neighborType='k';               % Type of neighbor.Choice:1)'k';Choice:2)'epsilon'
options.neighborPara=10;                % parameter for choosing the neighbor. number of neighbor for "k" type and radius for 'epsilon'
options.metric='euclidean';             % Method of measurement. Metric

% % Isomap PreImage options
% isomap.Reoptions.ReCoverNeighborType='k';% Type of neighbor of new point. Choice:1)'k';Choice:2)'epsilon'
% isomap.Reoptions.ReCoverNeighborPara=10;       % Parameter of neighbor of new point
% isomap.Reoptions.Recoverd2pType='Dw';          % Type of distance to coordinate method. Distance weight/Least square estimate
% isomap.Reoptions.Recoverd2pPara=1;             % Parameter of distance to coordinate recover method

%% Data Generation Swiss Roll Dataset
t1=rand(Num_data,1)*4*pi;   % Theater
t2=rand(Num_data,1)*20;
t1=sort(t1);                
X(:,1)=t1.*cos(t1);         % X
X(:,2)=t2;                  % Y
X(:,3)= t1.*sin(t1);        % Z
color = t1;                     % Color according to Theater
size = ones(Num_data,1)*10;    % Size. Constant

    
%% Isomaps on X    
[Z,model] = Isomaps(X,options);
% [model2] = Kpca_orig(X',options);
% Z2=model2.Z';


%% Ploting
figure(1)
scatter3(X(:,1),X(:,2),X(:,3),size,color);
title('Original dataset')

figure(2)
scatter(Z(:,1),Z(:,2),size,color);
title(sprintf('Projection with %d principal components',new_dim))




