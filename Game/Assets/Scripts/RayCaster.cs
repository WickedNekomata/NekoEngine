using System.Collections;
using JellyBitEngine;

public class RayCaster : JellyScript
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
            Debug.Log("My ray has " + ray.position.ToString() + "as position and " + ray.direction.ToString() + "as direction.");

            RaycastHit hitInfo;

            if(Physics.Raycast(ray, out hitInfo, 3000f, (uint)layerMask.masks, true, true))
            {
                Debug.Log("Dude! I raycasted " + hitInfo.gameObject.name + "!");
            }
        }    
    }
}

