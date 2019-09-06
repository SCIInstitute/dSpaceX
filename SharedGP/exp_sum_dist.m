%given log(a1),...,log(an)
%return a1/sum(a1+...+an), ..., an/sum(a1+...+an)
function dist = exp_sum_dist(log_elems)
    max_elems = max(log_elems);
    elems = exp(log_elems - max_elems);
    sum_all = sum(elems);
    dist = elems/sum_all;
end