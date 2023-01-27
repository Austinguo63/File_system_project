#include "student.h"


int smallest(int array[], int length) {  
  int x, smallest_e;
  int postive;
  //int final;

  for (x = 0; x <length; x++){  // start a loop.
    if (length == 1){
      if (array[0] <0){
        postive = array[0] * (-1);  // if array only have one number, and it will be the smallest one.
        return postive;  // just make this number positive if <0. 
      }
    }
    else {
      if( array[x] <0){
        array[x]= array[x]*(-1);  // else, array have two or more numbers, if current x is <0, make it positive.
      }
      if (array[x] < smallest_e){  // if current x is smaller, update smallest number.
        smallest_e = array[x];
      }
    }
  }
  return smallest_e;  
  return 0;
}

double mean(int array[], int length) { 
  int x, sum;
  for (x =0; x<length; x++){  // use a loop
    sum = sum + array[x];  // add all number together as a whole number(sum).
  }
  
  return sum/length;  //use this total / number of numbers in array, equal mean.
  return 0;
}

void swap(int *a, int *b) {  

  int c; 
  c = *a;  // use this c as a temprary place to put value in pointer a. 
  *a = *b;  // then assign value inside *b to *a.
  *b = c;  // now make *b to value store in c, which is equal to origenal *a. 
  //swap finshed

}

void squaredPrimes(int array[],int length){  
  int x;
  for (x=0; x<length; x++){  //use a loop
    
    if (array[x]==2){
      array[x] = array[x] * array[x];  // 2 is a spcial case, it is prime. so squard it.
    }
    //else if any number modulo 2 ,3,5 and 7; left a whole number, it will not be a primer number. just change nothing from array.
    else if (array[x]%2 ==0 || array[x]%3 ==0 || array[x]%5 ==0 || array[x]%7 ==0){ 
      array[x] = array[x];
    }
    else {
      array[x] = array[x]*array[x];  // everything else in array will be prime, and need to be squard.
    }
  }
}

void sort(int array[],int length){ 
  int x,y;

  for (x=0; x<length; x++ ){  // for loop to set x as first number in array.

    for (y =x+1; y<length; y++){ // another for loop but start with next number in array from x.

      if (array[x]>array[y]){ // if array[x] is larger than it next number in this array.

        swap(&array[x], &array[y]); // use swap function to change position to make bigger number go right. 

      }
    }
  }
    return;
  

}



int square(int n){   // this is a square function to check if a number n can be squared or not.
  int x;
  for (x =1; x*x<=n ; x++ ){ // for x start from 1, and limit x with x^2 can not go larger than n, looping. 
    if ((n / x == x ) && (n % x ==0) ){  // ex: 9/3=3 and 9%3 ==0, 9 is able to do squared.
      // if there is a number x that fit the require, then this tested n will be able to squared. 
      return 1; // return true
    }
  }
return 0;

}

void negate_fibonacci(int fibarray[],int fibLength){
  int y;
  for (y = 0; y<fibLength; y++){  // start a for loop
    if (square(5 * (fibarray[y] * fibarray[y]) +4) || square( 5 * (fibarray[y] * fibarray[y]) -4)){  // by the formula 5w^2+4 or 5w^2-4 is perfect square, any number from fibarray[] fit in this condition will be fib number. 
      fibarray[y] = fibarray[y] *(-1);  // make this number go negtive by times -1.
    }
    else {
      fibarray[y] = fibarray[y];  // else everynumber form array just stay same. 
    }

    
  }
  
}
  
  
  