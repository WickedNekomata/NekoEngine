using System;
using System.Collections;
using JellyBitEngine;

public class Alita : JellyScript
{
    //Alita propeties
    int life = 50;
    int damage = 20;
    float speed = 10.0f;
    float acceleration = 30.0f;

    //Habilities
    Dash dash = null;

    // Raycast
    public LayerMask terrainMask = new LayerMask();
    public LayerMask enemyMask = new LayerMask();

    //Place to go
    Vector3 placeToGo = new Vector3(0, 0, 0);

    //Agent
    private NavMeshAgent agent = null;

    //Particles
    //ParticleEmitter smoke = null;

    //Alita states
    private enum Alita_State
    {
        IDLE,
        RUNNING,
        GOING_TO_ATTK,
        ATTK,
        DASHING,
        AREA_ATTK
    }
    Alita_State state = Alita_State.IDLE;

    //Enemy
    GameObject enemy = null;
    Unit enemy_unit = null;

    //Variables about attack distance and time
    public float attack_dist = 2.0f;
    public float attk_period = 1.0f;
    private float attk_cool_down = 0.0f;

    //Variables for SP attacks
    private bool isAreaActive = false;
    public GameObject areaCircle;
    public float circleRadius = 5.0f;

    public override void Awake()
    {
        agent = gameObject.GetComponent<NavMeshAgent>();
        dash = gameObject.GetComponent<Dash>();

    }

    //Called every frame
    public override void Update()
    {

        CheckState();

        if(state != Alita_State.DASHING)
            CheckForMouseClick();

        if (state == Alita_State.IDLE || state == Alita_State.RUNNING)
            CheckForDash();

        if (state != Alita_State.ATTK && state != Alita_State.AREA_ATTK && state != Alita_State.DASHING)
            CheckForSPAttack(); //Only special attacks when no normal attacking
    }


    //Check for... functions
    //---------------------------------------------------------------------------------------------------------------------------------

    private void CheckState()
    {
        switch (state)
        {
            case Alita_State.IDLE:
                
                break;
            case Alita_State.RUNNING:

                float dist = (float)(placeToGo - transform.position).magnitude;
                if (dist <= 0.1f)
                {
                    agent.SetDestination(transform.position);
                    state = Alita_State.IDLE;
                }

                break;
            case Alita_State.GOING_TO_ATTK:

                float diff = (float)(enemy.transform.position - transform.position).magnitude;
                if (diff <= attack_dist + 1.0f)
                {
                    Debug.Log("ARRIVE TO ENEMY");
                    agent.SetDestination(transform.position);
                    state = Alita_State.ATTK;
                }

                break;
            case Alita_State.ATTK:
                if (enemy != null)
                    NormalAttack();

                else
                {
                    enemy = null;
                    enemy_unit = null;
                    state = Alita_State.IDLE;
                }
                break;
            case Alita_State.DASHING:
                bool dashing = true;
                dashing = dash.CheckForDashEnd(agent);

                if(!dashing)
                    state = Alita_State.IDLE;
                break;
            case Alita_State.AREA_ATTK:
                agent.SetDestination(transform.position);
                AreaAttack();
                state = Alita_State.IDLE;
                break;
        }
    }

    private void CheckForMouseClick()
    {
        //Attack
        if (Input.GetMouseButtonDown(MouseKeyCode.MOUSE_LEFT))
            DecideIfGoToAttack();

        //Move
        if (Input.GetMouseButton(MouseKeyCode.MOUSE_RIGHT))
            Move();
           
    }


    private void CheckForDash()
    {
        if (Input.GetKeyDown(KeyCode.KEY_SPACE))
        {
            bool dashing = false;
            dashing = dash.ExecuteDash(agent);

            if (dashing)
            {
                state = Alita_State.DASHING;
                areaCircle.active = false;
                isAreaActive = false;
            }
        }
    }

    private void CheckForSPAttack()
    {
        if (Input.GetKeyDown(KeyCode.KEY_Q))
        {
            if (!isAreaActive)
            {
                areaCircle.active = true;
                isAreaActive = true;
                Debug.Log("ACIVATE AREA");
            }

            else if (isAreaActive)
            {
                areaCircle.active = false;
                isAreaActive = false;
                Debug.Log("DESACTIVATE AREA");
            }
        }

        if (Input.GetMouseButton(MouseKeyCode.MOUSE_LEFT) && isAreaActive)
        {
            state = Alita_State.AREA_ATTK;
            isAreaActive = false;
            areaCircle.active = false;

        }
    }

    //Basic Actions
    //---------------------------------------------------------------------------------------------------------------------------------

    private void Move()
    {
        Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
        RaycastHit hit;
        if (Physics.Raycast(ray, out hit, float.MaxValue, terrainMask, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
        {
            state = Alita_State.RUNNING;
            agent.maxSpeed = speed;
            agent.maxAcceleration = acceleration;

            if (agent != null)
            {
                Debug.Log("GOING TO SPOT");
                agent.SetDestination(hit.point);
                placeToGo = hit.point;
            }
            else
                Debug.Log("AGENT IS NULL");

        }
    }

    private void DecideIfGoToAttack()
    {
        Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
        RaycastHit hit;
        if (Physics.Raycast(ray, out hit, float.MaxValue, enemyMask, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
        {
            //Go to attack
            enemy = hit.gameObject;
            if (enemy != null)
            {
                state = Alita_State.GOING_TO_ATTK;
                agent.maxSpeed = speed;
                agent.maxAcceleration = acceleration;

                //Determine a place a little further than enemy position
                Vector3 enemy_fwrd_vec = (transform.position - enemy.transform.position).normalized();
                Vector3 enemy_pos = enemy.transform.position + enemy_fwrd_vec * attack_dist;

                agent.SetDestination(enemy_pos);

                enemy_unit = enemy.GetComponent<Unit>();

                Debug.Log("GOING TO ENEMY");
            }
            else
                Debug.Log("ENEMY IS NULL");
        }
    }

    //Attacks
    //---------------------------------------------------------------------------------------------------------------------------------

    private void NormalAttack()
    {
        attk_cool_down += Time.deltaTime;

        //Attack every second
        if (attk_cool_down >= attk_period)
        {
            if (enemy != null && enemy_unit != null)
            {
                enemy_unit.Hit(damage);
                Debug.Log("ENEMY HIT");
            }

            attk_cool_down = 0.0f;

            if (!Input.GetMouseButton(MouseKeyCode.MOUSE_LEFT))
                state = Alita_State.IDLE;
        }
    }


    private void AreaAttack()
    {
        Debug.Log("AREA ATTACK!!!!!");

        OverlapHit[] hitInfo;
        if (Physics.OverlapSphere(circleRadius, transform.position, out hitInfo, enemyMask, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
        {
            foreach (OverlapHit hit in hitInfo)
            {
                hit.gameObject.GetComponent<Unit>().Hit(damage); //Not change this
                Debug.Log("HIT ENEMY: " + hit.gameObject.name);
            }
        }

    }

}
