using System.Collections;
using JellyBitEngine;
using JellyBitEngine.UI;

public class Activator : JellyScript
{
    bool imageActive = true;
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
        if(Input.GetKeyDown(KeyCode.KEY_1))
        {
            Image image = gameObject.GetComponent<Image>();
            image.SetActive(!imageActive);
            imageActive = !imageActive;
        }
    }
}

