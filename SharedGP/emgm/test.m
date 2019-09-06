clear all;
close all;

%load data;
data = load('/home/shandian/Research/pyInfTucker/swg-botp-anon-lower-action.mat');
X = data.data';
[label, r] = emgm(X,20); 
csvwrite('/home/shandian/Research/pyInfTucker/swg-botp-anon-lower-action.csv',r);
%spread(x,label);