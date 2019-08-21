function [Z,eigvals,model] = KernelIsomap(X,option)

    % local parameter
    Metric='euclidean';
% % %     num_neighbor=7;
    num_neighbor = option.neighbor;
    
    %----------------------------------------------------------------------
    
    [num,dim]=size(X);
    
    % Euclidean distance calculations
    D_E=pdist2(X,X,Metric);     %Distance_Euclidean % Matlab statistic Tool box
    
    
    % Step 1: Construct neighborhood graph  % Matrix of Distance_neighbor
     [D_E_softed, D_E_soind] = sort(D_E); 
     D_N=D_E;   
     for i=1:num
         temp_coli=D_E_softed(:,i);         % coli = column i th
         temp_coli(2+num_neighbor:end)=inf;
         D_N(D_E_soind(:,i),i)=temp_coli;
     end
     D_N=D_N';
     D_N = min(D_N,D_N');       %% Important
     
% % %      D_N=tril(D_N,-1)+triu(D_N',1);
     
     % Using Floyds algorithm to compute the shortest path to each point
     D_F = FastFloyd(D_N);
     
     % Since the direction is invertible. make the matrix being delivered
     % to classic MDS symmetric.
     D_F_mirror=tril(D_F,-1)+triu(D_F',1);
     
     
     %-----------------------------------------------------------------
     % The kernel part of isomap -------------- MDS function is not used as
     % it is integrated.
      
     D=D_F_mirror;
     [Dim_D,~]=size(D);
     H = eye(Dim_D)-ones(Dim_D,Dim_D)/Dim_D;  % H is the Centering matrix
     K_Dsqr= (-0.5)*H*(D.^2)*H;
     K_D= (-0.5)*H*(D)*H;
     
     K_c=[zeros(Dim_D),2*K_Dsqr;-1*eye(Dim_D),-4*K_D];
     
     Eval=eig(K_c);
     Eval=sort(Eval);
     c_star=Eval(1);
     
     K_wave=K_Dsqr+2*c_star*K_D+c_star^2/2*H;
      
     %--------------------------------------------

     
    [Evec,Eval]=eig(K_wave);  
    %---------------------------------------------------------------------
    % Ensure the value of eigenvalue is real %%% need to be consider twice
    Evec=real(Evec);
    Eval=real(Eval);
    
    %---------------------------------------------------------------------
    % reorganize the eigenvalue in decreasing order (including minus value)
    % the program finds and offers the right order
    vals=diag(Eval);
    [~,order]=sort(vals);
    order = order(end:-1:1); %make the order decrease rather than increase
    
    % % %     vals(order)              %show eigrnvalue
    %---------------------------------------------------------------------
    % Recording & Output
    % full imformation
    
    
    model.X=X; 
    
    model.eigval=vals(order);
    model.eigvec=Evec(:,order);
    
    val=diag(model.eigval);
    vec=Evec(:,order);
    model.Z_full=vec*sqrt(val);
    
    % results of dimension reduction
    dim_new=option.dim_new;
    eigvals=model.eigval(1:dim_new);
    val=diag(model.eigval(1:dim_new));
    Z=vec(:,1:dim_new)*sqrt(val);

    
end