function [y_pred] = pred_cigplvm(model, uTe)

    N = size(model.U,1);
    
    %% for all training data

    Sigma = 1/model.bta*eye(N) + ker_func(model.U,model.ker_params); 
    Knn = ker_cross(uTe, model.U, model.ker_params);
    y_pred = Knn*(Sigma\model.yTr);
        
end

