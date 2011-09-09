typedef struct client
{
  char nick[20];
  int sock;
}
client;

typedef struct LNode
{
  client  e;
  struct LNode *next;
}
LNode;

typedef struct LList
{
  LNode *head;
  int Length;
}
LList;

void InitList(LList *L)
{
  L->head=(LNode *)malloc(sizeof(LNode));
  L->Length=0;
}

int ListInsert(LList *L,client e)
{
  LNode *s;
  s=(LNode *)malloc(sizeof(LNode));
  s->e=e;
  s->next=L->head->next;
  L->head->next=s;
  L->Length++;
  return 0;
}

int ListDelete(LList *L,int sock)
{
  LNode *q=L->head;
  LNode *p=q;
  while(q!=0&&q->e.sock!=sock)
  {
    p=q;
    q=q->next;
  }
  if (q!=0&&q->e.sock!=sock) return -1;
  p->next=q->next;
  free(q);
  L->Length--;
}
