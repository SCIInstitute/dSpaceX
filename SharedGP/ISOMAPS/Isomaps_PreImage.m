function X_star = Isomaps_PreImage(Z_star,model,options)
% function of Isomap dimension reduction preimage solution.       MARK-II
%
% Synopsis:
% X_star = Isomaps_PreImage(Z_star,model)
% X_star = Isomaps_PreImage(Z_star,model,options)
%
% Description:
% The function find a new point's position in original dataspace using the
% offered position information in embedded space with local linear
% interpretation method. See dist2pos.m function.
% 
% steps for the program:
% 
% Input:
% Z_star [Samples X Dimensions]         The new point in embedded space
% model  [structure]
%      .X                       % Original dataset
%      .D_N                     % Neighbor Distance Matrix
%      .eigvals                 % Corresponding Eigenvalue 
%      .Z                       % Same as Z
%      .options [struct] 
%              .dim_new         % New dimension in embedded space
%              .neighborType    % Type of neighbor
%              .neighborPara    % Parameter of neighbor
%              .metric          % Method of measurement. Metric
%
% options [structure]           % Options for the Pre-image solution      
%        .neighborType   % Type of neighbor of new point. Choice:1)'k';Choice:2)'epsilon'
%        .neighborPara   % Parameter of neighbor of new point
%        .type        % Type of distance to coordinate method. Distance weight/Least square estimate
%        .para       % Parameter of distance to coordinate recover method
%
% Output:
% X_star   [samples x dimension_new] Coordinate of X_star in original data
%                                    space, corresponding to Z_star.
%
% Pcakage Require:
% Example:
%
% See also 
% MDS; Isomaps
% 
% About: 
% 
% Modifications:
% 30-Aug-2014, WeiX, first edition 
% WeiX, Dec-1th-2014, Update
% WeiX, Orc-27th-2015, Add local basis method

%% ----------------------Initialization------------------------------------
[Num_Zstar,Dim_Zstar]=size(Z_star);
[Num_X,Dim_X]=size(model.X);
X_star=zeros(Num_Zstar,Dim_X);          % Assign memory

if nargin < 3, options = []; end
if ~isfield(options,'neighborType'), options.neighborType = model.options.neighborType; end
if ~isfield(options,'neighborPara'), options.neighborPara = model.options.neighborPara; end 
if ~isfield(options,'type'), options.type ='Exp'; end                % Default reconstruction method
% if ~isfield(options,'para'), options.para = 2; end                   % Default kernel function

%% -----------------------Main--------------------------------
for i = 1:Num_Zstar    
    % ----Calculate all the distances between Z_stari and all Z-----
%     disti=pdist2(model.Z,Z_star(i,:));
    disti=pdist2(model.Z(:,1:Dim_Zstar),Z_star(i,:));
        
    % ----Find neighbor of Z_star and the distances accoring to the choosen method.
    
    %Identify neighbors
    switch options.neighborType
        case 'k'
            Num_Neighbor=options.neighborPara;  % Parameter
            % --find the cloest points, their coordinate and the cloest distances
            [disti_soft,index]=sort(disti);
            X_base    = model.X(index(1:Num_Neighbor),:);
            Z_base    = model.Z(index(1:Num_Neighbor),:);
            dist_base = disti_soft(1:Num_Neighbor);

        case 'epsilon'
            radius=options.neighborPara;         % Parameter
            Num_Neighbor=sum((disti<=radius));
            [disti_soft,index]=sort(disti);
            X_base    = model.X(index(1:Num_Neighbor),:);
            Z_base    = model.Z(index(1:Num_Neighbor),:);
            dist_base = disti_soft(1:Num_Neighbor);

        otherwise
            error('Error: Undefined type of neighbor in reconstruction.');    

    end

    %Check number of neighbor
    if Num_Neighbor==0
        error('number of neighbor is 0. Reconstructions can not be done')
    end
    
%     % OLD METHOD
%     %Distance to coordinates
%     switch options.type
%         case 'LB'
%             Z_base = model.Z(index(1:Num_Neighbor),:);
%             V=LocalBasis( X_base', Z_base' );
%             X_star(i,:)=Z_star(i,:)*V';
% 
%         otherwise
%             X_star(i,:)= Dist2pos(X_base,dist_base,options);
% 
%     end
    
    switch options.type
        case 'LpcaI'
%             options.InMethod = 'LSE';          		 % Default Interpretation method;  
%             options.dim_new=10;
            X_star(i,:)= Pos2PosLpcaI(X_base,Z_base,dist_base',Z_star(i,:),options);
        otherwise
            X_star(i,:)= Dist2pos(X_base,dist_base,options);
    end
    
    
    


end


end


