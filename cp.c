#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ------------------------------ AVL Node ------------------------------
typedef struct AVLNode {
    char itemId[50];
    char itemName[50];
    char currentBid[50];
    int height;
    struct AVLNode *left;
    struct AVLNode *right;
} AVLNode;

// ------------------------------ Global Declarations ------------------------------
HWND hwndMain, hwndItemId, hwndItemName, hwndCurrentBid, hwndNewBid, hwndListBox;
AVLNode *root = NULL;
int serialNumber = 1;

// ------------------------------ AVL Tree Functions ------------------------------
AVLNode *newNode(char itemId[], char itemName[], char currentBid[]) {
    AVLNode *node = (AVLNode *)malloc(sizeof(AVLNode));
    strcpy(node->itemId, itemId);
    strcpy(node->itemName, itemName);
    strcpy(node->currentBid, currentBid);
    node->left = node->right = NULL;
    node->height = 1;
    return node;
}

int height(AVLNode *node) {
    return node ? node->height : 0;
}

int balanceFactor(AVLNode *node) {
    return node ? height(node->left) - height(node->right) : 0;
}

AVLNode *rightRotate(AVLNode *y) {
    AVLNode *x = y->left;
    AVLNode *T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = 1 + (height(y->left) > height(y->right) ? height(y->left) : height(y->right));
    x->height = 1 + (height(x->left) > height(x->right) ? height(x->left) : height(x->right));
    return x;
}

AVLNode *leftRotate(AVLNode *x) {
    AVLNode *y = x->right;
    AVLNode *T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = 1 + (height(x->left) > height(x->right) ? height(x->left) : height(x->right));
    y->height = 1 + (height(y->left) > height(y->right) ? height(y->left) : height(y->right));
    return y;
}

void insert(AVLNode **root, char itemId[], char itemName[], char currentBid[]) {
    if (*root == NULL) {
        *root = newNode(itemId, itemName, currentBid);
        return;
    }

    int cmp = strcmp(itemId, (*root)->itemId);
    if (cmp == 0) return;
    if (cmp < 0) insert(&(*root)->left, itemId, itemName, currentBid);
    else insert(&(*root)->right, itemId, itemName, currentBid);

    (*root)->height = 1 + (height((*root)->left) > height((*root)->right) ? height((*root)->left) : height((*root)->right));
    int balance = balanceFactor(*root);

    if (balance > 1 && strcmp(itemId, (*root)->left->itemId) < 0)
        *root = rightRotate(*root);
    else if (balance < -1 && strcmp(itemId, (*root)->right->itemId) > 0)
        *root = leftRotate(*root);
    else if (balance > 1 && strcmp(itemId, (*root)->left->itemId) > 0) {
        (*root)->left = leftRotate((*root)->left);
        *root = rightRotate(*root);
    } else if (balance < -1 && strcmp(itemId, (*root)->right->itemId) < 0) {
        (*root)->right = rightRotate((*root)->right);
        *root = leftRotate(*root);
    }
}

AVLNode *search(AVLNode *root, char itemId[]) {
    if (!root || strcmp(root->itemId, itemId) == 0) return root;
    return strcmp(itemId, root->itemId) < 0 ? search(root->left, itemId) : search(root->right, itemId);
}

void preorderList(AVLNode *node, HWND hwndListBox) {
    if (!node) return;

    char display[200];
    sprintf(display, "Item ID: %s | Name: %s | Bid: %s", node->itemId, node->itemName, node->currentBid);
    SendMessage(hwndListBox, LB_ADDSTRING, 0, (LPARAM)display);

    preorderList(node->left, hwndListBox);
    preorderList(node->right, hwndListBox);
}

// ------------------------------ Bidding Logic ------------------------------
void updateBid(AVLNode *node, char newBid[], HWND hwnd) {
    if (!node) return;

    int current = atoi(node->currentBid);
    int newValue = atoi(newBid);

    if (newValue > current) {
        sprintf(node->currentBid, "%d", newValue);
        SetWindowText(hwndCurrentBid, newBid);

        FILE *file = fopen("auction_data.txt", "a");
        if (file == NULL) {
            perror("File open error");
            MessageBox(hwnd, "File write error", "Error", MB_OK);
            return;
        }
        fprintf(file, "Updated Bid - Item: %s, New Bid: %s\n", node->itemId, node->currentBid);
        fclose(file);

        MessageBox(hwnd, "Bid updated!", "Success", MB_OK);
    } else {
        MessageBox(hwnd, "New bid must be higher than current.", "Invalid", MB_OK);
    }
}

// ------------------------------ Windows GUI Functions ------------------------------
void AddControls(HWND hwnd) {
    CreateWindow("STATIC", "Item ID:", WS_VISIBLE | WS_CHILD, 20, 20, 100, 20, hwnd, NULL, NULL, NULL);
    hwndItemId = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 20, 150, 20, hwnd, NULL, NULL, NULL);

    CreateWindow("STATIC", "Item Name:", WS_VISIBLE | WS_CHILD, 20, 50, 100, 20, hwnd, NULL, NULL, NULL);
    hwndItemName = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 50, 150, 20, hwnd, NULL, NULL, NULL);

    CreateWindow("STATIC", "Current Bid:", WS_VISIBLE | WS_CHILD, 20, 80, 100, 20, hwnd, NULL, NULL, NULL);
    hwndCurrentBid = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 80, 150, 20, hwnd, NULL, NULL, NULL);

    CreateWindow("STATIC", "New Bid:", WS_VISIBLE | WS_CHILD, 20, 110, 100, 20, hwnd, NULL, NULL, NULL);
    hwndNewBid = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 110, 150, 20, hwnd, NULL, NULL, NULL);

    CreateWindow("BUTTON", "Add Item", WS_VISIBLE | WS_CHILD, 20, 150, 100, 30, hwnd, (HMENU)1, NULL, NULL);
    CreateWindow("BUTTON", "Place Bid", WS_VISIBLE | WS_CHILD, 130, 150, 100, 30, hwnd, (HMENU)2, NULL, NULL);
    CreateWindow("BUTTON", "Show Items", WS_VISIBLE | WS_CHILD, 240, 150, 100, 30, hwnd, (HMENU)3, NULL, NULL);

    hwndListBox = CreateWindow("LISTBOX", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOTIFY, 20, 200, 440, 140, hwnd, NULL, NULL, NULL);
}

void AddItem(HWND hwnd) {
    char itemId[50], itemName[50], currentBid[50];
    GetWindowText(hwndItemId, itemId, sizeof(itemId));
    GetWindowText(hwndItemName, itemName, sizeof(itemName));
    GetWindowText(hwndCurrentBid, currentBid, sizeof(currentBid));

    if (search(root, itemId)) {
        MessageBox(hwnd, "Item already exists!", "Error", MB_OK);
        return;
    }

    insert(&root, itemId, itemName, currentBid);

    FILE *file = fopen("auction_data.txt", "a");
    if (!file) {
        perror("Error opening file");
        MessageBox(hwnd, "Could not save item.", "Error", MB_OK);
        return;
    }
    fprintf(file, "Item #%d\nID: %s\nName: %s\nBase Bid: %s\n\n", serialNumber++, itemId, itemName, currentBid);
    fclose(file);

    MessageBox(hwnd, "Item added successfully!", "Success", MB_OK);
}

void PlaceBid(HWND hwnd) {
    char itemId[50], newBid[50];
    GetWindowText(hwndItemId, itemId, sizeof(itemId));
    GetWindowText(hwndNewBid, newBid, sizeof(newBid));

    AVLNode *node = search(root, itemId);
    if (!node) {
        MessageBox(hwnd, "Item not found!", "Error", MB_OK);
        return;
    }

    updateBid(node, newBid, hwnd);
}

void ShowItems(HWND hwnd) {
    SendMessage(hwndListBox, LB_RESETCONTENT, 0, 0);
    preorderList(root, hwndListBox);
}

// ------------------------------ WinMain and WndProc ------------------------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        AddControls(hwnd);
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 1: AddItem(hwnd); break;
        case 2: PlaceBid(hwnd); break;
        case 3: ShowItems(hwnd); break;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "AuctionApp";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);
    hwndMain = CreateWindow("AuctionApp", "Auction Bidding System", WS_OVERLAPPEDWINDOW, 100, 100, 500, 400, NULL, NULL, hInstance, NULL);
    ShowWindow(hwndMain, nCmdShow);
    UpdateWindow(hwndMain);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
