function [Z,model] = MDS(D,dim_new,options)
% function of multi dimention scalling  (MARK-II)
%
% Synopsis:
% [Z,model] = MDS(D,dim_new)
% [Z,model] = MDS(D,dim_new,options)
%
% Description:
% multi dimention scalling is a way of describing some points, among which
% the distances are known,  in acoordinate system so that such a
% describtion preserves all distances as accurate as poiisble.
% 
% steps for the program:
% 
% Input:
%  D        [samples x samples] indicates distance matrix. All distances are saved here.
%  dim_new  scale value         indicates the dimension of the new system
%
% Output:
% Z         [samples x dimension_new] indicates embedded dataset 
% eigvals1  corresponding eigenvalue of Z
% model     some record parameter for after use
%
% Pcakage Require: 
%
% See also 
% Isomap;
% 
% About: 
% 
% Modifications:
% 29-Aug-2014, WeiX, first edition 
% WeiX, Dec 3rd 2014, Update



%% Initialization and Parameters
start_time = cputime;
[num,~]=size(D);

if nargin < 3, options = []; end
if ~isfield(options,'FullRec'), options.FullRec = 0; end      

%% Multi Dimension Scalling
% -----Double centering-------
H = eye(num)-ones(num,num)/num;         % H is the Centering matrix
D_Dc = (-0.5)*H*(D.^2)*H;               % D_DoubleCentering

% -----eigen decomposion-----
[Evec,Eval]=eig(D_Dc);

% -----Ensure the value of eigenvalue is real------------------------------
% This is necessary becaus of the calculation limit of MATLAB. It sometimes
% generates results such as 0.00000+0.00000i. This would cause problem for 
% the followed process.
Evec=real(Evec);
Eval=real(Eval);

% % --this is a correction with warning--
%
% if (isreal(Evec)==0)
%     warning('Eigenvector of Distance_DoubleCentering matrix is complex. Auto Correction Activate')
%     Evec=real(Evec);
% end
% 
% if (isreal(Evec)==0)
%     warning('Eigenvvalue of Distance_DoubleCentering matrix is complex. Auto Correction Activate')
%     Eval=real(Eval);
% end


% ----Find the best eigenvalue and arrange them in decreasing order-------
% reorganize the eigenvalue in decreasing order (including minus value)
% the program finds and offers the right order
vals=diag(Eval);         %vals is a vector
[~,order]=sort(vals);
order = order(end:-1:1); %make the order decrease rather than increase
    

% ----Recording & Output----------------
% --full imformation stored in model--
model.D=D;
% model.eigval=vals(order);
% model.eigvec=Evec(:,order);

val=diag(vals(order));
vec=Evec(:,order);
% 
% eigvals=model.eigval(1:dim_new);
% val=diag(model.eigval(1:dim_new));

Z=vec(:,1:dim_new)*sqrt(val(1:dim_new,1:dim_new));
     
% model.Z_full=vec*sqrt(val);
% ------------------------Save the model-----------------------------------
model.DR_method='MDS';
model.D = D;
model.eigenvalues=vals(order);
model.cputime = cputime - start_time;
% model.Z=Z;

% Full Information. Conditional output. (For Further Research without recalculation. Mass Memoey required)
if options.FullRec == 1
%     model.Z_full=vec*sqrt(val);
%     model.eigenval=diag(vals(order));              
%     model.eigenvec=Evec(:,order);
    model.eigenval=val;              
    model.eigenvec=vec;   
    model.cputime = cputime - start_time;     % Update process time
end

end

