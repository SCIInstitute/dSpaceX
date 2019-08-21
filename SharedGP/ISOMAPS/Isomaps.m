function [Z,model] = Isomaps(X,options)
% Isomap dimension reduction.  
%
% Synopsis:
% Isomap(X);
% Isomap(X,options);
%
% Description:
% Apply Isomap to embed dataset X to a embeded space Z.
% 
% Input:
%  X        [samples x dimension_X] indicates Training input data X.
%  options [struct] 
%         .dim_new              % New dimension
%         .neighborType         % Type of neighbor.Choice:1)'k';Choice:2)'epsilon'
%         .neighborPara         % parameter for choosing the neighbor.
%                                 number of neighbor for "k" type and radius for 'epsilon'
%         .metric               % Method of measurement. Metric
%
% Output:
% Z         [samples x dimension_new] indicates embedded dataset
% model
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
% Pcakage Require: Matlab statistic Tool box(pdist2);
%
% See also 
% MDS; 
% 
% Modifications:
% 27-Aug-2014, WeiX, first edition 
% WeiX, Dec 3rd 2014, Update
% WeiX, 4-1-2016, Structure optimize to reduce size of model
%
%
%% Initialization and Parameters
start_time = cputime;
[num,dim]=size(X);

if nargin < 2, options = []; end
if ~isfield(options,'neighborType'), options.neighborType = 'k'; end
if ~isfield(options,'neighborPara'), options.neighborPara = num/10; end % 10% of point as neighbor point;
if ~isfield(options,'new_dim'), options.new_dim = 2; end                % Default new dimension=3;
if ~isfield(options,'metric'), options.metric ='euclidean'; end         % Default metric to measure the distance;
if ~isfield(options,'FullRec'), options.FullRec = 0; end      


%% Main
% ---------------------Construct the Graph---------------------------------
 D_E=pdist2(X,X,options.metric);     %Distance_Euclidean % Matlab statistic Tool box

switch options.neighborType
    case 'k'
         num_neighbor=options.neighborPara;
         [D_E_sorted, D_E_sortIndex] = sort(D_E); 
         D_N=D_E;                     % Distance_Neighbor
         for i=1:num
             temp_coli=D_E_sorted(:,i);         % coli = column i th
             temp_coli(2+num_neighbor:end)=inf;
             D_N(D_E_sortIndex(:,i),i)=temp_coli;
         end
         D_N=D_N';
         D_N = min(D_N,D_N');       %% Important. Ensure the matrix is symmetric
         
         clearvars temp_coli        % clean temp variable
         
    case 'epsilon'
        radius=options.neighborPara;
        D_N =  D_E./(D_E<=radius); 
%       D = min(D,INF); 
    otherwise
        error('Error: Undefined type of neighbor.');    
 
end

% ------------------------Find the shortest path---------------------------
% Using Floyds algorithm to compute the shortest path to each point.
% (Dijkstra's algorithm is significantly more efficient for sparse graphs.)
D_F = FastFloyd(D_N);       % Distance_Floyds

% ------------------------Check Connection of Graph------------------------
if (sum(isinf(D_F(:)))>=1)   % if there are any 'inf' element in the matrix
%     warning('Graph is not connected. Isomap could not ne carried out. Please increase neighbor/radius. Press any key to continue')
%     pause
    error('Graph is not connected. Isomap could not be carried out. Please increase neighbor/radius.')
end

% ------------------------Ensure symmetric---------------------------------
% Since the direction is invertible. make the matrix being delivered
% to classic MDS is symmetric.
D_F_mirror=tril(D_F,-1)+triu(D_F',1);

% ------------------------Classic MDS--------------------------------------
[Z,model_mds] = MDS(D_F_mirror,options.dim_new);


% ------------------------Save the model-----------------------------------
% model.options.dim_new
% model.options.neighborType
% model.options.neighborPara
% model.options.metric

model.DR_method='Isomaps';
model.options = options;
model.X = X;
model.eigenvalues=model_mds.eigenvalues;
model.Z=Z;

model.cputime = cputime - start_time;

% Full Information. Conditional output. (For Further Research without recalculation. Mass Memoey required)
if options.FullRec == 1 
    model.D_N=D_N;

    model.eigenval=model_mds.eigenval;              
    model.eigenve= model_mds.eigenvec;   
    model.cputime = cputime - start_time;     % Update process time
end


    
end