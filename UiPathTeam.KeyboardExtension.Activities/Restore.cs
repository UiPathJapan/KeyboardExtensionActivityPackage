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
            if (WindowHandle.Expression == null)
            {
                Bridge.StopAgent((IntPtr)0); // means all windows previously applied
            }
            else
            {
                var value = WindowHandle.Get(context);
                if (value == null)
                {
                    throw new ArgumentException(Resource.WindowHandleArgumentValidationError);
                }
                Bridge.StopAgent(Bridge.ToWin32Handle(value));
            }
        }
    }
}
