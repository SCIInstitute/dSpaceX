% MDS_PreImage_Demo
% Demonstration of PreImage solver on MDS 
%
% Instructions:
% 1) Change the parameter section to see performance of Isomaps_PreImage.
% Tips: 
% For the classic MDS Preimage it would always fail to provide meaningful
% reconstruction unless use all 3 dimension, which is not dimension
% reduction at all. OR unless the data is basically 2D.(make t2=rand(Num_data,1)*1 in line 31);
%
% Modifications:
% WeiX, lost date,    first edition 
% WeiX, Dec-3rd-2014, Add comment

%% Initialization
clear

%% parameters
Num_data=200;        
dim_new=2;

options.type='Dw';
options.para=2;
options.neighbor=10;


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
% Here it is a euclidean base classic MDS
D =pdist2(X,X,'euclidean');
[Z,model] = MDS(D,dim_new);
model.X=X;
X_star = MDS_PreImage(Z,Z,X,options);

for index =1:Num_data
    Z_star=Z(index,:);    
    Z_temp=removerows(Z,'ind',index);  % Z_temp is model without the index point to avoide 100% accurate recovery.
    X_temp=removerows(X,'ind',index);    
    X_star(index,:) = MDS_PreImage(Z_star,Z_temp,X_temp,options);
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
