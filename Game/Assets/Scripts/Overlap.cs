using System.Collections;
using JellyBitEngine;

public class Overlap : JellyScript
{
    //Use this method for initialization
    public override void Awake()
    {

    }

    //Called every frame
    public override void Update()
    {
        if(Input.GetKeyDown(KeyCode.KEY_1))
        {
            OverlapHit[] hitInfo;
            if(Physics.OverlapSphere(100f, new Vector3(0,0,0), out hitInfo, LayerMask.GetMask("Default"), SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
            {
                Debug.Log("Overlap");
            }

        }
    }
}

