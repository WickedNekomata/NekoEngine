using System.Collections;
using JellyBitEngine;

public class Dash : JellyScript
{
    //Dash propeties
    public float dashLength = 7.0f;
    public float stopRadius = 3.0f;
    public float maxDashSpeed = 200.0f;
    public float maxDashAcc = 200.0f;
    public float coolDownTime = 0.2f;

    private bool coolDown = false;
    private float coolDwnTimr = 0.0f;

    //Place to go
    private Vector3 placeToGo = new Vector3(0, 0, 0);

    // Raycast
    public LayerMask terrainMask = new LayerMask();
    

    public override void Awake()
    {

    }


    public bool ExecuteDash(NavMeshAgent agent)
    {
        bool dashing = false;

        Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
        RaycastHit hit;

        if (Physics.Raycast(ray, out hit, float.MaxValue, terrainMask, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
        {
            agent.maxSpeed = maxDashSpeed;
            agent.maxAcceleration = maxDashAcc;

            Vector3 mouse_pos = hit.point;

            if ((mouse_pos - transform.position).magnitude < stopRadius)
                mouse_pos = new Vector3(mouse_pos.x * stopRadius, mouse_pos.y, mouse_pos.z * stopRadius);

            Vector3 direction = (mouse_pos - transform.position).normalized();
            Vector3 dash_pos = transform.position + direction * dashLength;

            agent.SetDestination(dash_pos);
            placeToGo = dash_pos;

            dashing = true;
        }
        return dashing;
    }

    public bool CheckForDashEnd(NavMeshAgent agent)
    {
        bool dashing = true;

        float distDash = (float)(placeToGo - transform.position).magnitude;
        if (distDash <= stopRadius)
        {
            agent.maxSpeed = 0.0f;
            coolDown = true;
        }

        if (coolDown)
            dashing = CoolingDown();

        return dashing;
    }

    private bool CoolingDown()
    {
        coolDwnTimr += Time.deltaTime;

        if (coolDwnTimr >= coolDownTime)
        {
            Debug.Log("COOLED");
            coolDwnTimr = 0.0f;
            coolDown = false;
            return false;
        }

        return true;
    }

}