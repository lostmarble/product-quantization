void print_array(int** a) {
    int i = 0,j=0;
    for (i=0;i<2;i++) {
        for (j=0;j<2;j++) {
            printf("%d ",a[i][j]);
        }
    }

}

int main() {
    int a[4][2];
    int i = 0,j=0;
    for (i=0;i<4;i++) {
        for (j=0;j<2;j++) {
            a[i][j]=i+j;
            printf("%d ",a[i][j]);
            printf("\n");
        }
    }

    print_array(&(a[0]));
    print_array(&(a[2]));
    return 0;

}




