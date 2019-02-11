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
            
            RaycastHit hitInfo;

            if(Physics.Raycast(ray, out hitInfo, float.MaxValue, (uint)layerMask.masks, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
            {
                Debug.Log("I raycasted " + hitInfo.gameObject.name);
                hitInfo.gameObject.transform.position += Vector3.forward * Time.deltaTime * 6f;
            }
        }    
    }
}

