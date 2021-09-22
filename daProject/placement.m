

filename=uigetfile('*.txt');
delimiterIn='\t';
A = importdata(filename,'\t', 0);
B=size(A);
for k=[1:B(1,1)]
x=A(k,1);

y=A(k,2);
disp(y);
rectangle('position',[x y 4 16]);
hold on ;
end

