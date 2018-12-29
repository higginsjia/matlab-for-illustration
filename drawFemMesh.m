%%
%{
Description: Finite element method course project of FEM model illustration 
Code language: MATLAB
E-mail:zhangyj@hust.edu.cn
Date:2018/12/29
%}
clc;clear;
nNode=25;
nElem=32;
X=zeros(1,nNode);
Y=zeros(1,nNode);
for i=1:nNode
    X(i)=mod(i-1,5);
end
Y(1:5)=0;
Y(6:10)=1;
Y(11:15)=2;
Y(16:20)=3;
Y(21:25)=4;
scatter(X,Y);
%element number and node number
%first column represents element index and other columns represent node
%index for triangle element
tElem=[
 1     1     2    6;
 2     2     3    7;
 3     3     4    8;
 4     4     5    9;
 5     2     7    6;
 6     3     8    7;
 7     4     9    8;
 8     5    10    9;
9      6     7    11;
10     7     8    12;
11     8     9    13;
12     9     10   14;
13     7     12   11;
14     8     13   12;
15     9     14   13;
16    10     15   14;
17     11    12   16;
18     12    13   17;
19     13    14   18;
20    14     15   19;
21     12    17   16;
22     13    18   17;
23     14    19   18;
24    15     20   19;
25    16    17    21;
26    17    18    22;
27    18    19    23;
28    19     20   24;
29    17     22   21;
30    18     23   22;
31    19     24   23;
32    20     25   24
];
nodeIdx=tElem(:,2:4);
triplot(nodeIdx,X,Y,'linewidth',1.5,'color','k');
hold on;
scatter(X,Y,'b','filled');
for i=1:nElem
    px=(X(nodeIdx(i,1))+X(nodeIdx(i,2))+X(nodeIdx(i,3)))/3;
    py=(Y(nodeIdx(i,1))+Y(nodeIdx(i,2))+Y(nodeIdx(i,3)))/3;
    text(px,py,['E',num2str(i)]);
end
for i=1:nNode
    px=X(i);
    py=Y(i);
    text(px+0.05,py+.1,['N' num2str(i)]);
end
%draw x along tag
dex=0.1;
for i=0:4
    
    px=i;
    py=0.0;
    x=[px,px+dex,px-dex];
    y=[py,py-dex,py-dex];
    patch(x,y,[0,0.8,1]);
    hold on;
end
for i=0:4
    px=0;
    py=i;
    x=[px,px-dex,px-dex];
    y=[py,py-dex,py+dex];
    patch(x,y,'g');
    hold on;
end
%%set legend
px=1.0;
py=-0.2;
x=[px,px+dex,px-dex];
y=[py,py-dex,py-dex];
patch(x,y,'g');

text(px+0.2,py,'x向约束');
px=px+1.0;
x=[px,px+dex,px-dex];
y=[py,py-dex,py-dex];
patch(x,y,[0,0.8,1]);
text(px+0.2,py,'y向约束');
%for arrors 
x=0:4;
y=4*ones(1,5);
u=zeros(1,5);
v=ones(1,5);
quiver(x,y,u,v,0.2,'linewidth',2,'color','k');

fValue=[500 1000 1000 1000 500];
for i=0:4
    text(i+0.05,4.3,['F' num2str(i+1) ' (0,' num2str(fValue(i+1)) ')' ]);
end

xlabel('x');
ylabel('y');
axis([-0.5,4.8,-0.5,4.5]);
saveas(gcf,'fem2d.jpg');


