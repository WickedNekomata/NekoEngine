using System.Collections;
using JellyBitEngine;

public class attack : JellyScript
{
    //Alita propeties
    int life = 50;
    int damage = 20;
  
    // Raycast
    private RaycastHit hit;
    private int terrainMask = LayerMask.GetMask("Default");
    private int enemyMask = LayerMask.GetMask("Default");

    //Agent
    private NavMeshAgent agent = null;

    //Alita states
    private enum Alita_State
    {
        IDLE,
        RUNNING,
        GOING_TO_ATTK,
        ATTK,
        AREA_ATTK
    }
    Alita_State state = Alita_State.IDLE;

    //Enemy
    GameObject enemy;
    Unit enemy_unit;

    //Variables about attack distance and time
    public float attack_dist = 2.0f;
    public float attk_period = 1.0f;
    private float attk_cool_down = 0.0f;

    public override void Start()
    {
        agent = gameObject.GetComponent<NavMeshAgent>();
    }

    //Called every frame
    public override void Update()
    {
        if (agent == null)
            agent = gameObject.GetComponent<NavMeshAgent>();

        CheckState();
        CheckForMouseClick();
    }

    private void CheckState()
    {
        switch (state)
        {
            case Alita_State.IDLE:

                break;
            case Alita_State.RUNNING:

                if (hit.point == transform.position)
                    state = Alita_State.IDLE;

                break;
            case Alita_State.GOING_TO_ATTK:

                float diff = (float)(enemy.transform.position - transform.position).magnitude;
                if (diff <= attack_dist + 1.0f)
                {
                    Debug.Log("ARRIVE TO ENEMY");
                    state = Alita_State.ATTK;
                }
                break;
            case Alita_State.ATTK:

                if (enemy != null)
                    NormalAttack();
                else
                    state = Alita_State.IDLE;
                break;

            case Alita_State.AREA_ATTK:

                break;
        }
    }


    private void CheckForMouseClick()
    {
        //Attack
        if (Input.GetMouseButton(MouseKeyCode.MOUSE_LEFT))
        {
            Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
            if (Physics.Raycast(ray, out hit, float.MaxValue, (uint)enemyMask, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
            {
                //Go to attack
                state = Alita_State.GOING_TO_ATTK;
                enemy = hit.gameObject;

                //Determine a place a little further than enemy position
                Vector3 enemy_fwrd_vec = (transform.position - enemy.transform.position).normalized();
                Vector3 enemy_pos = enemy.transform.position + enemy_fwrd_vec * attack_dist;
                agent.SetDestination(hit.point);

                enemy_unit = enemy.GetComponent<Unit>(); /////HERE GET ANOTHER SCRIPT MAYBE???? I DON'T KNOW xd

                Debug.Log("GOING TO ENEMY");

            }

        }

        //Move
       if (Input.GetMouseButton(MouseKeyCode.MOUSE_RIGHT))
        {
            Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
            if (Physics.Raycast(ray, out hit, float.MaxValue, (uint)terrainMask, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
            {
                if (agent != null)
                {
                    Debug.Log("GOING TO SPOT");
                    agent.SetDestination(hit.point);
                }
                else
                    Debug.Log("AGENT IS NULL");
            }
        }
    }

    private void NormalAttack()
    {
        attk_cool_down += Time.deltaTime;

        //Attack every second
        if (attk_cool_down >= attk_period)
        {
            enemy_unit.Hit(damage);
            Debug.Log("ENEMY HIT");

            attk_cool_down = 0.0f;
        }
    }

}