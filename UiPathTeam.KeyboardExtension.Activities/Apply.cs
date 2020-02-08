using System;
using System.Activities;

namespace UiPathTeam.KeyboardExtension.Activities
{
    [LocalizedDisplayName(nameof(Resource.ApplyDisplayName))]
    [LocalizedDescription(nameof(Resource.ApplyDescription))]
    public class Apply : CodeActivity
    {
        [LocalizedCategory(nameof(Resource.InputCategory))]
        [LocalizedDisplayName(nameof(Resource.ApplyWindowHandleDisplayName))]
        [LocalizedDescription(nameof(Resource.ApplyWindowHandleDescription))]
        [RequiredArgument]
        public InArgument<object> WindowHandle { get; set; }

        [LocalizedCategory(nameof(Resource.OutputCategory))]
        [LocalizedDisplayName(nameof(Resource.ApplyResultDisplayName))]
        [LocalizedDescription(nameof(Resource.ApplyResultDescription))]
        public OutArgument<bool> Result { get; set; }

        protected override void Execute(CodeActivityContext context)
        {
            bool res = false;
            if (WindowHandle != null)
            {
                var value = WindowHandle.Get(context);
                if (value != null)
                {
                    res = Bridge.StartAgent(Bridge.ToWin32Handle(value));
                }
            }
            if (Result != null)
            {
                Result.Set(context, res);
            }
        }
    }
}
