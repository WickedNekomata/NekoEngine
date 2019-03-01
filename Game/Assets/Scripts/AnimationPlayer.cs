using System.Collections;
using JellyBitEngine;

public class AnimationPlayer : JellyScript
{
    //Use this method for initialization
    public override void Awake()
    {
        
    }

    //Called every frame
    public override void Update()
    {
        if (Input.GetKeyDown(KeyCode.KEY_1))
        {
            Destroy(gameObject);
            Animator animator = gameObject.GetComponent<Animator>();
            if (animator != null)
            {
                animator.PlayAnimation(0);
            }
        }
    }
}

