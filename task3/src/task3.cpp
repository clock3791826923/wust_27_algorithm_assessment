#include<iostream>
#include<vector>
#include<queue>
std::vector<int> dr={1,-1,0,0};
std::vector<int> dc={0,0,1,-1};
int min_g[100][100];
struct node
{
    int x,y,g,h,f;
    bool operator<(const node &a) const//小顶堆要对<号重载
    {
        return f>a.f;
    }
};
//深度优先搜索：底层为栈
void dfs(int x,int y,char a[100][100],bool (&b)[100][100],int r,int c,int &step)
{
    if (x<0 || x>=r || y<0 || y>=c || a[x][y]=='#' || b[x][y]==true)
    {
        return;
    }
    b[x][y]=true;
    step++;
    for (int i=0; i<4; i++)
    {
        dfs(x+dr[i],y+dc[i],a,b,r,c,step);
    }
}
int geth(int x,int y,int zr,int zc)//得到哈夫曼距离
{
    return abs(x-zr)+abs(y-zc);
}
void astar(char a[100][100],int qr,int qc,int zr,int zc,int r,int c)
{
    for (int i=0; i<r; i++)
    {
        for (int j=0; j<c; j++)
        {
            min_g[i][j]=1e9;
        }
    }
    std::priority_queue<node> pq;
    int start_h=geth(qr,qc,zr,zc);
    pq.push({qr,qc,0,start_h,start_h});
    min_g[qr][qc]=0;//初始化起点
    while (!pq.empty())
    {
        node m=pq.top();
        pq.pop();
        if (m.x==zr && m.y==zc)
        {
            std::cout<<"cost: "<<m.g<<std::endl;
            return;
        }
        if (m.g>min_g[m.x][m.y])
        {
            continue;
        }
        for (int i=0; i<4; i++)
        {
            int nx=m.x+dr[i];
            int ny=m.y+dc[i];
            if (nx<0 || nx>=r || ny<0 || ny>=c || a[nx][ny]=='#')
            {
                continue;
            }
            int mc=0;
            if (a[nx][ny]=='.')
            {
                mc=1;
            }
            else if (a[nx][ny]>='2' && a[nx][ny]<='9')
            {
                mc=a[nx][ny]-'0';
            }
            int new_g=m.g+mc;
            if (new_g<min_g[nx][ny])
            {
                min_g[nx][ny]=new_g;
                int new_h=geth(nx,ny,zr,zc);
                pq.push({nx,ny,new_g,new_h,new_g+new_h});
            }
        }
    }
}
int main()
{
    int r,c,qr,qc,zr,zc;
    char a[100][100];
    std::cout<<"请分别输入行、列"<<std::endl;
    std::cin>>r>>c;
    for (int i=0; i<r; i++)
    {
        for (int j=0; j<c; j++)
        {
            std::cin>>a[i][j];
            if (a[i][j] == 'S')
            {
                qr=i;
                qc=j;
                a[i][j]='.';
            }
            if (a[i][j]=='G')
            {
                zr=i;
                zc=j;
                a[i][j]='.';
            }
        }
    }
    int step=0;
    bool visited[100][100]={false};
    dfs(qr,qc,a,visited,r,c,step);//深度优先搜索
    std::cout<<"reachable_cells: "<<step<<std::endl;
    //step=0;
    //bfs(qr,qc,a,visited,r,c,step);//广度优先搜索
    //std::cout<<"reachable_cells: "<<step<<std::endl;
    int sum=1;
    for (int i=0; i<r; i++)
    {
        for (int j=0; j<c; j++)
        {
            if (visited[i][j]==false && a[i][j]=='.')
            {
                step=0;
                dfs(i,j,a,visited,r,c,step);
                sum++;
            }
        }
    }
    std::cout<<"regions: "<<sum<<std::endl;
    astar(a,qr,qc,zr,zc,r,c);//A*算法
    return 0;
}