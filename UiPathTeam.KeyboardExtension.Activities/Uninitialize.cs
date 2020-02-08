using System.Activities;

namespace UiPathTeam.KeyboardExtension.Activities
{
    [LocalizedDisplayName(nameof(Resource.UninitializeDisplayName))]
    [LocalizedDescription(nameof(Resource.UninitializeDescription))]
    public class Uninitialize : CodeActivity
    {
        protected override void Execute(CodeActivityContext context)
        {
            Bridge.StopServer();
        }
    }
}
