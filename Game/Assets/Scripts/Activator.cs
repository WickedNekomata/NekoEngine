using System.Collections;
using JellyBitEngine;
using JellyBitEngine.UI;

public class Activator : JellyScript
{
    bool imageActive = true;
    public LayerMask LayerMask = new LayerMask();
    public enum TEST
    {
        CALABASA,
        ARKAXOFA,
        SIDA,
        MATENME
    }

    public TEST testEnum;

    //Use this method for initialization
    public override void Awake()
    {
       
    }

    //Called every frame
    public override void Update()
    {
        if(Input.GetKeyDown(KeyCode.KEY_1))
        {
            testEnum.GetType();
            System.Enum.GetValues(typeof(TEST));
            System.Enum.GetName(typeof(TEST), 1);
            Image image = gameObject.GetComponent<Image>();
            image.SetActive(!imageActive);
            imageActive = !imageActive;
        }
    }
}

