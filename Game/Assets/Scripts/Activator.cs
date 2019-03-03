using System.Collections;
using JellyBitEngine;

public class Activator : JellyScript
{
    public LayerMask LayerMask = new LayerMask();
    public enum TEST
    {
        ONE,
        TWO,
        THREE
    }

    public TEST testEnum = TEST.THREE;

    //Use this method for initialization
    public override void Awake()
    {
        testEnum = TEST.ONE;
    }

    //Called every frame
    public override void Update()
    {
       
    }
}

