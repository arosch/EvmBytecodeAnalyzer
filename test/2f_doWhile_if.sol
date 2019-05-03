pragma solidity ^0.5;

contract Addition {

    int left =3;

    int right = 5;
    int count = 20;

    function add(int a) public view returns(int result){
      int i=a;
      do{
        if(i==right){
          return 5;
        }
        i++;
      }while(i<count);
      result=i;
    }

    function sub() public{
    right =6;
    }
}
