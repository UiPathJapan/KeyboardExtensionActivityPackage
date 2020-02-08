using System.Activities;

namespace UiPathTeam.KeyboardExtension.Activities
{
    [LocalizedDisplayName(nameof(Resource.InitializeDisplayName))]
    [LocalizedDescription(nameof(Resource.InitializeDescription))]
    public class Initialize : CodeActivity
    {
        protected override void Execute(CodeActivityContext context)
        {
            Bridge.StartServer();
        }
    }
}
