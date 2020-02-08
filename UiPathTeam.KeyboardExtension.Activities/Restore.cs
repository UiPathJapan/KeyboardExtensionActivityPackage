using System;
using System.Activities;

namespace UiPathTeam.KeyboardExtension.Activities
{
    [LocalizedDisplayName(nameof(Resource.RestoreDisplayName))]
    [LocalizedDescription(nameof(Resource.RestoreDescription))]
    public class Restore : CodeActivity
    {
        [LocalizedCategory(nameof(Resource.InputCategory))]
        [LocalizedDisplayName(nameof(Resource.RestoreWindowHandleDisplayName))]
        [LocalizedDescription(nameof(Resource.RestoreWindowHandleDescription))]
        public InArgument<object> WindowHandle { get; set; }

        protected override void Execute(CodeActivityContext context)
        {
            if (WindowHandle != null)
            {
                var value = WindowHandle.Get(context);
                if (value != null)
                {
                    Bridge.StopAgent(Bridge.ToWin32Handle(value));
                    return;
                }
            }
            Bridge.StopAgent((IntPtr)0);
        }
    }
}
