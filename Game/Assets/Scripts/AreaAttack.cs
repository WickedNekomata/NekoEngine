using System.Collections;
using JellyBitEngine;

public class AreaAttack : JellyScript
{
    //Alita propeties
    int life = 50;
    int damage = 20;

    // Raycast
    private RaycastHit hit;
    private uint terrainMask = LayerMask.GetMask("Default");
    private uint enemyMask = LayerMask.GetMask("Default");

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

    //Area attack variables
    bool isAreaActive = false;
    GameObject circleArea = null;

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

        if(state != Alita_State.ATTK && state != Alita_State.AREA_ATTK)
            CheckForSPAttack(); //Only special attacks when no normal attacking

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
                agent.SetDestination(transform.position);
                AreaAttk();
                state = Alita_State.IDLE;
                break;
        }
    }


    private void CheckForMouseClick()
    {
        //Attack
        if (Input.GetMouseButton(MouseKeyCode.MOUSE_LEFT))
        {
            Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
            if (Physics.Raycast(ray, out hit, float.MaxValue, enemyMask, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
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
            if (Physics.Raycast(ray, out hit, float.MaxValue, terrainMask, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
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

    private void CheckForSPAttack()
    {
        if (Input.GetKeyDown(KeyCode.KEY_Q))
        {
            if (!isAreaActive)
            {
                //Instance circle
                isAreaActive = true;
            }

            else if (isAreaActive)
            {
                //Destroy circle
                isAreaActive = false;
            }

        }


        if (Input.GetMouseButton(MouseKeyCode.MOUSE_LEFT) && isAreaActive)
        {
            state = Alita_State.AREA_ATTK;
            isAreaActive = false;
            //Destroy circle
        }

    }

    private void NormalAttack()
    {
        attk_cool_down += Time.deltaTime;

        //Attack every second
        if (attk_cool_down >= attk_period)
        {
            enemy_unit.Hit(damage);
            Debug.Log("ENEMY HIT"); //CHANGE THIS I THINK

            attk_cool_down = 0.0f;
        }
    }

    public void AreaAttk()
    {
        float circleRadius = 100.0f;

        OverlapHit[] hitInfo;
        if (Physics.OverlapSphere(circleRadius, transform.position, out hitInfo, enemyMask, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
        {
            foreach (OverlapHit hit in hitInfo)
            {
                hit.gameObject.GetComponent<Unit>().Hit(damage); //CHANGE THIS I THINK
                Debug.Log("HIT ENEMY: " + hit.gameObject.name);
            }
        }
    }
 }

