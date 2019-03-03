using System.Collections;
using JellyBitEngine;

public class AreaAtkkBtn : JellyScript
{
    public GameObject go_at = null;
    private AreaAttk script_areaAttak = null;
    //Use this method for initialization
    public override void Awake()
    {

    }

    //Called every frame
    public override void Update()
    {

    }           

    public void OnClick()
    {
        if (go_at != null)
            if (script_areaAttak == null)
                script_areaAttak = go_at.GetComponent<AreaAttk>();

        //script_areaAttak.
    }
}

