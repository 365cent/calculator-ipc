/*============================================================
| Large integer multiplication can be achieved by decomposing the two factors and performing multiplication and addition on the simplified terms. This program uses the Karatsuba algorithm to multiply two large integers.
| Usage: ./calculator <4 digit integer> <4 digit integer>
| Description of variables:
|   factors: the two factors to be multiplied
|   result: the result of the multiplication
|   DEBUG: the debug mode
|   LENGTH: the length of the input(restiction mode)
|   n: the digit of the input
|   fdi: input pipe for child
|   fdo: output pipe for child
|   buffer: buffer for pipe
|   pid: the id of the child process
|   abcd: the four parts of the factors
|   xyz: the three parts of the product
|   a1: the first half of the first factor
|   a2: the second half of the first factor
|   b1: the second half of the second factor
|   b2: the second half of the second factor
|----------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#define DEBUG 0  // set to 1 to enable debug messages
#define LENGTH 4 // define it other than 4 for unrestrict input length

int multiple(int factors[2]);
int powTen(int n);
int logTen(int n);

int main(int argc, char *argv[])
{
  int factors[2], result = 0;
  // Check if the user has entered 2 numbers
  if (argc != 3)
  {
    printf("Proper usage is ./calculator <4 digit integer> <4 digit integer>\n");
    return 1;
  }
  // Check if the user has entered 0
  if (!strcmp(argv[1], "0") || !strcmp(argv[2], "0"))
  {
    printf("%s * %s = 0\n", argv[1], argv[2]);
    return 0;
  }
  // Remove leading 0's
  while (argv[1][0] == '0')
  {
    argv[1]++;
  }
  while (argv[2][0] == '0')
  {
    argv[2]++;
  }
  // Check if both numbers are of the same length
  if (strlen(argv[1]) != strlen(argv[2]))
  {
    printf("Please enter number have same length.\n");
  }
  // Check if the user has entered a non-numeric character
  if (!atoi(argv[1]) || !atoi(argv[2]))
  {
    printf("Invalid input.\n");
    return 1;
  }
#if LENGTH == 4
  if (strlen(argv[1]) != 4 || strlen(argv[2]) != 4)
  {
    printf("Proper usage is ./calculator <4 digit integer> <4 digit integer>\n");
    return 1;
  }
#endif
  factors[0] = atoi(argv[1]);
  factors[1] = atoi(argv[2]);
  printf("Your integers are %d %d\n", factors[0], factors[1]);
  result = multiple(factors);
  printf("%d\n", result);
  return 0;
}

// powTen is a alternative to pow10() function in math.h
int powTen(int n)
{
  int result = 1;
  while (n--)
  {
    // result = result * 8 + result * 2
    result = (result << 3) + (result << 1);
  }
  return result;
}

// logTen() is a alternative to log10() function in math.h
int logTen(int n)
{
  int result = 0;
  while (n)
  {
    n /= 10;
    result++;
  }
  return result;
}

int multiple(int factors[2])
{
  int fdi[2], fdo[2], buffer[2], xyz[3], abcd[4], n = logTen(factors[0]), a1, a2, b1, b2, result;
  pid_t pid;
  if (n <= 1)
  {
    return factors[0] * factors[1];
  }
  // Partition a,b into a = a1 * 10^(n/2) + a2 and b = b1 * 10^(n/2) + b2
  a1 = factors[0] / powTen(n / 2), a2 = factors[0] % powTen(n / 2), b1 = factors[1] / powTen(n / 2), b2 = factors[1] % powTen(n / 2);
#if DEBUG
  printf("n = %d, a1 = %d, a2 = %d, b1 = %d, b2 = %d\n", n, a1, a2, b1, b2);
#endif
  pipe(fdi);
  pipe(fdo);
  pid = fork();
  switch (pid)
  {
  case -1:
    perror("fork");
    exit(1);
  case 0:
    close(fdi[1]);
    close(fdo[0]);
    for (int i = 0; i < 4; i++)
    {
      if (read(fdi[0], buffer, sizeof(buffer)) == -1)
      {
        perror("read");
        exit(1);
      }
      else
      {
        printf("\tChild (PID %d) Received %d from parent\n", getpid(), buffer[0]);
        printf("\tChild (PID %d) Received %d from parent\n", getpid(), buffer[1]);
      }
      result = buffer[0] * buffer[1];
      if (write(fdo[1], &result, sizeof(result)) == -1)
      {
        perror("write");
        exit(1);
      }
      else
      {
        printf("\tChild (PID %d) Sending %d to parent\n", getpid(), result);
      }
    }
    close(fdi[0]);
    close(fdo[1]);
    exit(0);
  default:
    close(fdi[0]);
    close(fdo[1]);
    printf("Parent (PID %d): created child (PID %d)\n", getpid(), pid);
    printf("\n###\n# Calculating X\n###\n");
    memcpy(buffer, (int[]){a1, b1}, sizeof(buffer));
    if (write(fdi[1], buffer, sizeof(buffer)) == -1)
    {
      perror("write");
      exit(1);
    }
    else
    {
      printf("Parent (PID %d) Sending %d to child\n", getpid(), buffer[0]);
      printf("Parent (PID %d) Sending %d to child\n", getpid(), buffer[1]);
    }
    if (read(fdo[0], &abcd[0], sizeof(abcd[0])) == -1)
    {
      perror("read");
      exit(1);
    }
    else
    {
      printf("Parent (PID %d) Received %d from child\n", getpid(), abcd[0]);
    }
    printf("\n###\n# Calculating Y\n###\n");
    memcpy(buffer, (int[]){a2, b1}, sizeof(buffer));
    if (write(fdi[1], buffer, sizeof(buffer)) == -1)
    {
      perror("write");
      exit(1);
    }
    else
    {
      printf("Parent (PID %d) Sending %d to child\n", getpid(), buffer[0]);
      printf("Parent (PID %d) Sending %d to child\n", getpid(), buffer[1]);
    }
    memcpy(buffer, (int[]){a1, b2}, sizeof(buffer));
    if (write(fdi[1], buffer, sizeof(buffer)) == -1)
    {
      perror("write");
      exit(1);
    }
    else
    {
      printf("Parent (PID %d) Sending %d to child\n", getpid(), buffer[0]);
      printf("Parent (PID %d) Sending %d to child\n", getpid(), buffer[1]);
    }
    // while child is calculating B and C, parent is calculating X
    xyz[0] = abcd[0] * powTen(n);
#if DEBUG
    printf("X = %d * 10^%d = %d\n", abcd[0], n, xyz[0]);
#endif
    if (read(fdo[0], &abcd[1], sizeof(abcd[1])) == -1)
    {
      perror("read");
      exit(1);
    }
    else
    {
      printf("Parent (PID %d) Received %d from child\n", getpid(), abcd[1]);
    }
    if (read(fdo[0], &abcd[2], sizeof(abcd[2])) == -1)
    {
      perror("read");
      exit(1);
    }
    else
    {
      printf("Parent (PID %d) Received %d from child\n", getpid(), abcd[2]);
    }
    printf("\n###\n# Calculating Z\n###\n");
    memcpy(buffer, (int[]){a2, b2}, sizeof(buffer));
    if (write(fdi[1], buffer, sizeof(buffer)) == -1)
    {
      perror("write");
      exit(1);
    }
    else
    {
      printf("Parent (PID %d) Sending %d to child\n", getpid(), buffer[0]);
      printf("Parent (PID %d) Sending %d to child\n", getpid(), buffer[1]);
    }
    // while child is calculating D, parent is calculating Y
    xyz[1] = (abcd[1] + abcd[2]) * powTen(n / 2);
#if DEBUG
    printf("Y = (%d + %d) * 10^%d = %d\n", abcd[1], abcd[2], n / 2, xyz[1]);
#endif
    if (read(fdo[0], &abcd[3], sizeof(abcd[3])) == -1)
    {
      perror("read");
      exit(1);
    }
    else
    {
      printf("Parent (PID %d) Received %d from child\n", getpid(), abcd[3]);
    }
    // parent is calculating Z
    xyz[2] = abcd[3];
#if DEBUG
    printf("Z = %d\n", xyz[2]);
#endif
    // parent is calculating result
    result = xyz[0] + xyz[1] + xyz[2];
    printf("\n%d * %d = %d + %d + %d = ", factors[0], factors[1], xyz[0], xyz[1], xyz[2]);
    wait(NULL);
    close(fdi[1]);
    close(fdo[0]);
    return result;
  }
}
