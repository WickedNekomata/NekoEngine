using System.Collections;
using JellyBitEngine;

public class Raycaster : JellyScript
{
    public LayerMask layerMask = new LayerMask();
     
    //Use this method for initialization
    public override void Awake()
    {

    }

    //Called every frame
    public override void Update()
    {
        if(Input.GetMouseButtonDown(MouseKeyCode.MOUSE_LEFT))
        {
            Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);

            RaycastHit hitInfo;

            if(Physics.Raycast(ray, out hitInfo, float.MaxValue, (uint)layerMask.masks, SceneQueryFlags.Static | SceneQueryFlags.Dynamic))
            {
                Debug.Log("I hitted " + hitInfo.gameObject.name + "!");
            }
        }
    }
}

