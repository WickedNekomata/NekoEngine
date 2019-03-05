using System.Collections;
using JellyBitEngine;
using System;

class MeleeNetmanAnimation : JellyScript
{
    public GameObject enemy;

    public float recoil_time = 1.0f;
    public float velocity_to_recoil = 1.0f;

    public float hit_time = 1.0f;
    public float velocity_to_hit = 2.0f;

    public float initial_pos_time = 1.0f;
    public float velocity_to_initial_pos = 1.0f;
    Vector3 initial_pos;
    public float offset = 0.5f;

    float actual_time = 0.0f;
    bool recoil_finish = false;
    bool hit_finish = false;

    Vector3 direction;

    public override void Awake()
    {
        initial_pos = transform.localPosition;
    }

    public override void Update()
    {
        direction = (enemy.transform.position - transform.position).normalized();

        if(!recoil_finish)
        {
            if(actual_time <= recoil_time)
            {
                transform.position -= direction * velocity_to_recoil * Time.deltaTime;
                actual_time += Time.deltaTime;
            }
            else
            {
                actual_time = 0.0f;
                recoil_finish = true;
            }
        }
        else if(!hit_finish)
        {
            if (actual_time <= initial_pos_time)
            {
                transform.position += direction * velocity_to_hit * Time.deltaTime;
                actual_time += Time.deltaTime;
            }
            else
            {
                actual_time = 0.0f;
                hit_finish = true;
            }
        }
        else
        {
            if (actual_time <= hit_time)
            {
                transform.position -= direction * velocity_to_hit * Time.deltaTime;
                actual_time += Time.deltaTime;
            }
            else
            {
                transform.localPosition = initial_pos;
                actual_time = 0.0f;
                recoil_finish = false;
                hit_finish = false;
            }
        }



    }

}

