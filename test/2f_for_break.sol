pragma solidity ^0.5;

contract Addition {

    int left =3;
    int right = 5;
    int count = 20;

    function add(int a) public view returns(int result){
      for(int i=0;i<count;i++){
        if(left==right){
          break;
        }
        a++;
      }
      return a;
    }

    function sub() public{
        right =6;
    }
}
