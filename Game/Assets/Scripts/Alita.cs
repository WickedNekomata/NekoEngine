using System.Collections;
using JellyBitEngine;

public class Alita : JellyScript
{
    //Masks
    public LayerMask mask = new LayerMask();
    private int terrainMask = LayerMask.GetMask("Terrain");
    private int enemyMask = LayerMask.GetMask("Enemy");

    //Unit script
    private Unit unit;

    // Raycast
    private RaycastHit hit;
    private bool ray_has_hit = false;

    private enum Alita_State
    {
        IDLE,
        RUNNING,
        GOING_TO_ATTK,
        ATTK,
        AREA_ATTK
    }

    //Enemy
    GameObject enemy;
    Unit enemy_unit;

    //Variables about attack distance and time
    public float attack_dist = 2.0f;
    public float attk_period = 1.0f;
    private float attk_cool_down = 0.0f;

    Alita_State state = Alita_State.IDLE;

    //Use this method for initialization
    public override void Awake()
    {

    }

    //Called every frame
    public override void Update()
    {
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
                    Debug.Log("Arrive to enemy");
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
                unit.MoveTo(enemy_pos);

                enemy_unit = enemy.GetComponent<Unit>();

                Debug.Log("GOING TO ENEMY");
            }

        }

        if (Input.GetMouseButton(MouseKeyCode.MOUSE_RIGHT))
        {
            Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
            if (Physics.Raycast(ray, out hit, float.MaxValue, (uint)terrainMask, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
            {
                unit.MoveTo(hit.point);
                state = Alita_State.RUNNING;

                Debug.Log("GOING TO SPOT");
            }
        }

    }

    private void NormalAttack()
    {
        attk_cool_down += Time.deltaTime;

        //Attack every second
        if (attk_cool_down >= attk_period)
        {
            enemy_unit.Hit(unit.damage);
            Debug.Log("ENEMY HIT");

            attk_cool_down = 0.0f;
        }
    }

    private RaycastHit GetMouseRayCastHit()
    {
        Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
        ray_has_hit = Physics.Raycast(ray, out hit, float.MaxValue, (uint)mask.masks, SceneQueryFlags.Dynamic | SceneQueryFlags.Static);

        return hit;
    }


}
