using System.Collections;
using JellyBitEngine;
using JellyBitEngine.UI;

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
            UI.UIHovered();

            ParticleEmitter emitter = gameObject.GetComponent<ParticleEmitter>();
            if (emitter != null)
            {
                emitter.Play();
                emitter.Stop();
            }
        }
    }
}

