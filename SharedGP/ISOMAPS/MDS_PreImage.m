function X_star = MDS_PreImage(Z_star,Z,X,options)
% Multi Dimension Scaling preImage soultion.
%
% Synopsis:
% X_star = MDS_PreImage(Z_star,X,model,options)
%
% Description:
% The function find a new point's position in original dataspace using the
% offered position information in embedded space.
%
%
%
% Input:
%  Z_star[num_star dim_dr]     % The new point in embedded space
%  Z     [num x dim_dr]        % coordinate in reduced space
%  X     [num x dim_orig]      % Original dataset
%  Oprions   [Structure]       % PreImage solver parameter
%       .type      % Method of reconstruction form distance to position/
%                  % Choose from 'LSE'(Least square estimate) or
%                  % 'Dw'(Distance weight).                Default: LSE
%       .para      % Paratmter for Distance weight method. Default: 1
%       .neighbor  % Number of distances used. Choosing starts from
%                  % the shortest length.              Default: All
% Output:
%  d [num x dim_orig]       Corresponding position of Z_star in original space
%
% Example:
% 
% See also 
% MDS
% 
% Modifications:
% 19-jul-2013, WeiX, first edition 
% WeiX, Dec-1th-2014, Update

%----------------------------------------   
%% Initialization and Parameters

[Num_Zstar,Dim_Zstar]=size(Z_star);
[~,Dim_X]=size(X);
[~,Dim_Z]=size(Z);
if (Dim_Z~=Dim_Zstar) error('Reduced Space does not match');end
X_star=zeros(Num_Zstar,Dim_X);  % Assign memory

%% Main
for i = 1:Num_Zstar   
    % Calculate all the distance
    disti=pdist2(Z,Z_star(i,:));
%     disti=disti+(disti==0)*1e-50;
    X_star(i,:)= Dist2pos(X,disti,options);
end


end